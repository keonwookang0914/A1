// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1LogChannels.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/CriticalSection.h"
#include "Logging/LogScopedVerbosityOverride.h"
#include "Misc/AutomationTest.h"
#include "Misc/Optional.h"
#include "Misc/OutputDevice.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/ScopeLock.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace A1LogChannelsTest
{
	struct FCapturedLine
	{
		FString Message;
		FName Category;
		ELogVerbosity::Type Verbosity = ELogVerbosity::NoLogging;
	};

	// 살아 있는 동안 GLog를 지나가는 로그 줄을 모은다.
	class FLogCapture : public FOutputDevice
	{
	public:
		FLogCapture()
		{
			GLog->AddOutputDevice(this);
		}

		virtual ~FLogCapture() override
		{
			GLog->RemoveOutputDevice(this);
		}

		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			FScopeLock Lock(&LinesLock);
			Lines.Add(FCapturedLine{ FString(V), Category, Verbosity });
		}

		// 버퍼를 거치지 않고 로그를 찍은 스레드에서 바로 받는다.
		virtual bool CanBeUsedOnMultipleThreads() const override
		{
			return true;
		}

		// 다른 스레드의 로그와 섞이지 않도록 Token이 든 줄만 돌려준다.
		TArray<FCapturedLine> FindLines(const TCHAR* Token) const
		{
			FScopeLock Lock(&LinesLock);
			return Lines.FilterByPredicate([Token](const FCapturedLine& Line)
				{
					return Line.Message.Contains(Token);
				});
		}

	private:
		mutable FCriticalSection LinesLock;
		TArray<FCapturedLine> Lines;
	};

	// 로그를 찍은 자리. 찍힌 줄번호는 LineBefore와 LineAfter 사이여야 한다.
	struct FCallSite
	{
		FString FunctionName;
		int32 LineBefore = 0;
		int32 LineAfter = 0;
	};

	// Token이 든 줄이 정확히 하나이고 카테고리, Verbosity, 메시지가 기대와 같은지 검사한다.
	// 기대하는 메시지는 "Prefix + 함수명(줄번호) + 공백 + Body"다. Verbosity를 비워 두면 Verbosity는 검사하지 않는다.
	// 오류 문구에는 로그 본문을 넣지 않는다. 본문이 AddExpectedMessage의 패턴과 일치하면 프레임워크가 그 오류를 예상된 것으로 삼킨다.
	void TestSingleLine(FAutomationTestBase& Test, const FLogCapture& Capture, const TCHAR* What, const TCHAR* Token, const FLogCategoryBase& Category, const TOptional<ELogVerbosity::Type>& Verbosity, const FString& Prefix, const FCallSite& CallSite, const FString& Body)
	{
		const TArray<FCapturedLine> Lines = Capture.FindLines(Token);
		if (Lines.Num() != 1)
		{
			Test.AddError(FString::Printf(TEXT("%s: expected 1 captured line but found %d"), What, Lines.Num()));
			return;
		}

		const FCapturedLine& Line = Lines[0];
		if (Line.Category != Category.GetCategoryName())
		{
			Test.AddError(FString::Printf(TEXT("%s: expected category %s but got %s"), What, *Category.GetCategoryName().ToString(), *Line.Category.ToString()));
		}

		if (Verbosity.IsSet() && Line.Verbosity != Verbosity.GetValue())
		{
			Test.AddError(FString::Printf(TEXT("%s: expected verbosity %s but got %s"), What, ::ToString(Verbosity.GetValue()), ::ToString(Line.Verbosity)));
		}

		for (int32 LineNumber = CallSite.LineBefore + 1; LineNumber < CallSite.LineAfter; ++LineNumber)
		{
			const FString Expected = FString::Printf(TEXT("%s%s(%d) %s"), *Prefix, *CallSite.FunctionName, LineNumber, *Body);
			if (Line.Message.Equals(Expected, ESearchCase::CaseSensitive))
			{
				return;
			}
		}

		Test.AddError(FString::Printf(TEXT("%s: message is not '<prefix><function>(<line>) <body>'"), What));
		Test.AddInfo(FString::Printf(TEXT("%s: captured '%s'"), What, *Line.Message));
	}

	struct FFakeNetContext;
	ENetMode GetLogContextNetMode(const FFakeNetContext* Context);

	// A1_NETLOG는 GetLogContextNetMode(this)로 넷모드를 구한다.
	// 넷모드를 직접 정해 주는 문맥을 this로 넘겨, 접속 없이도 넷모드마다 출력이 맞는지 확인한다.
	struct FFakeNetContext
	{
		ENetMode NetMode = NM_Standalone;
		FCallSite CallSite;

		void Emit()
		{
			CallSite.FunctionName = FString(__FUNCTION__);
			CallSite.LineBefore = __LINE__;
			A1_NETLOG(LogMap, Log, TEXT("A1NetLogTest-Value %d"), 7);
			CallSite.LineAfter = __LINE__;
		}
	};

	ENetMode GetLogContextNetMode(const FFakeNetContext* Context)
	{
		return Context->NetMode;
	}
} // namespace A1LogChannelsTest

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FA1LogMacroTest, "A1.LogChannels.A1_LOG", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FA1LogMacroTest::RunTest(const FString& Parameters)
{
	using namespace A1LogChannelsTest;

	// 실행 환경의 로그 수준 설정과 무관하게 Log까지만 통과시킨다.
	LOG_SCOPE_VERBOSITY_OVERRIDE(LogA1, ELogVerbosity::Log);
	LOG_SCOPE_VERBOSITY_OVERRIDE(LogA1Cliff, ELogVerbosity::Log);
	LOG_SCOPE_VERBOSITY_OVERRIDE(LogA1Tutorial, ELogVerbosity::Log);

	// Warning 로그는 테스트 실패로 집계되므로 예상된 것이라고 알려 둔다. Warning 이상으로 정확히 한 번 나오는지는 프레임워크가 확인한다.
	AddExpectedMessagePlain(TEXT("A1LogTest-Warning"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);

	const FString FunctionName(__FUNCTION__);
	FLogCapture Capture;

	// 서식 인자가 치환되고 "함수명(줄번호)"가 앞에 붙는다.
	const int32 FormatBefore = __LINE__;
	A1_LOG(LogA1, Log, TEXT("A1LogTest-Format %d %s %.1f"), 42, TEXT("text"), 1.5f);
	const int32 FormatAfter = __LINE__;
	TestSingleLine(*this, Capture, TEXT("format"), TEXT("A1LogTest-Format"), LogA1, ELogVerbosity::Log, FString(), { FunctionName, FormatBefore, FormatAfter }, TEXT("A1LogTest-Format 42 text 1.5"));

	// 가변 인자가 없어도 되고, 서식 치환은 한 번만 일어나 %%가 % 하나로 나온다.
	const int32 PlainBefore = __LINE__;
	A1_LOG(LogA1, Log, TEXT("A1LogTest-Plain 100%%"));
	const int32 PlainAfter = __LINE__;
	TestSingleLine(*this, Capture, TEXT("plain"), TEXT("A1LogTest-Plain"), LogA1, ELogVerbosity::Log, FString(), { FunctionName, PlainBefore, PlainAfter }, TEXT("A1LogTest-Plain 100%"));

	// 카테고리와 Verbosity를 그대로 넘긴다.
	const int32 DisplayBefore = __LINE__;
	A1_LOG(LogA1Tutorial, Display, TEXT("A1LogTest-Display"));
	const int32 DisplayAfter = __LINE__;
	TestSingleLine(*this, Capture, TEXT("display"), TEXT("A1LogTest-Display"), LogA1Tutorial, ELogVerbosity::Display, FString(), { FunctionName, DisplayBefore, DisplayAfter }, TEXT("A1LogTest-Display"));

	// 예상된 경고는 프레임워크가 출력 장치에 넘기기 전에 Verbosity를 Verbose로 낮추므로, 캡처한 Verbosity는 검사하지 않는다.
	const int32 WarningBefore = __LINE__;
	A1_LOG(LogA1Cliff, Warning, TEXT("A1LogTest-Warning"));
	const int32 WarningAfter = __LINE__;
	TestSingleLine(*this, Capture, TEXT("warning"), TEXT("A1LogTest-Warning"), LogA1Cliff, TOptional<ELogVerbosity::Type>(), FString(), { FunctionName, WarningBefore, WarningAfter }, TEXT("A1LogTest-Warning"));

	// 억제된 Verbosity는 출력하지 않고 인자도 평가하지 않는다.
	int32 EvaluationCount = 0;
	A1_LOG(LogA1, Verbose, TEXT("A1LogTest-Suppressed %d"), ++EvaluationCount);
	TestEqual(TEXT("suppressed line count"), Capture.FindLines(TEXT("A1LogTest-Suppressed")).Num(), 0);
	TestEqual(TEXT("suppressed argument evaluation count"), EvaluationCount, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FA1NetLogMacroTest, "A1.LogChannels.A1_NETLOG", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FA1NetLogMacroTest::RunTest(const FString& Parameters)
{
	using namespace A1LogChannelsTest;

	LOG_SCOPE_VERBOSITY_OVERRIDE(LogMap, ELogVerbosity::Log);

	struct FNetModeCase
	{
		const TCHAR* What;
		ENetMode NetMode;
		FString ExpectedTag;
	};

	const FNetModeCase Cases[] = {
		{ TEXT("standalone"), NM_Standalone, TEXT("STANDALONE") },
		{ TEXT("dedicated server"), NM_DedicatedServer, TEXT("SERVER") },
		{ TEXT("listen server"), NM_ListenServer, TEXT("SERVER") },
		{ TEXT("client"), NM_Client, FString::Printf(TEXT("CLIENT_ID%d"), static_cast<int32>(GPlayInEditorID)) },
		{ TEXT("unknown"), NM_MAX, TEXT("NONE") },
	};

	for (const FNetModeCase& Case : Cases)
	{
		FLogCapture Capture;

		FFakeNetContext Context;
		Context.NetMode = Case.NetMode;
		Context.Emit();

		TestSingleLine(*this, Capture, Case.What, TEXT("A1NetLogTest-Value"), LogMap, ELogVerbosity::Log, FString::Printf(TEXT("[%s] "), *Case.ExpectedTag), Context.CallSite, TEXT("A1NetLogTest-Value 7"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FA1LogContextNetModeTest, "A1.LogChannels.GetLogContextNetMode", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FA1LogContextNetModeTest::RunTest(const FString& Parameters)
{
	// 월드를 찾을 수 없는 문맥은 NM_MAX다.
	TestEqual(TEXT("null context"), static_cast<int32>(::GetLogContextNetMode(nullptr)), static_cast<int32>(NM_MAX));
	TestEqual(TEXT("object without a world"), static_cast<int32>(::GetLogContextNetMode(GetTransientPackage())), static_cast<int32>(NM_MAX));

	UWorld* World = nullptr;
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.World() != nullptr)
		{
			World = WorldContext.World();
			break;
		}
	}

	if (World == nullptr)
	{
		AddInfo(TEXT("No world in this context. Skipped the actor, component and world checks."));
		return true;
	}

	// 액터와 컴포넌트는 자신의 GetNetMode()로, 그 밖의 객체는 월드의 넷모드로 풀린다.
	AWorldSettings* Actor = World->GetWorldSettings();
	USceneComponent* Component = NewObject<USceneComponent>(Actor);

	TestEqual(TEXT("actor"), static_cast<int32>(::GetLogContextNetMode(Actor)), static_cast<int32>(Actor->GetNetMode()));
	TestEqual(TEXT("component"), static_cast<int32>(::GetLogContextNetMode(Component)), static_cast<int32>(Component->GetNetMode()));
	TestEqual(TEXT("world-based object"), static_cast<int32>(::GetLogContextNetMode(World)), static_cast<int32>(World->GetNetMode()));
	TestTrue(TEXT("world net mode is resolved"), ::GetLogContextNetMode(World) != NM_MAX);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
