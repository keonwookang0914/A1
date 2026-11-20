// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Maps/A1MasterRoom.h"
#include "ProceduralMeshComponent.h"
#include "A1RaiderRoom.generated.h"

class AA1CreatureBase;
class AA1ChestBase;
class AA1EquipmentBase;
class AA1RaiderBase;

//Spawned Room Type
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Rounge UMETA(DisplayName = "Rounge"),
	Storage UMETA(DisplayName = "Storage"),
	Container UMETA(DisplayName = "Container"),
	Master UMETA(DisplayName = "Master"),
	Bridge UMETA(DisplayName = "Bridge"),
	LBridge UMETA(DisplayName = "LBridge"),
	BedRoom UMETA(DisplayName = "BedRoom"),
	SecondFloor UMETA(DisplayName = "SecondFloor")
};

//Spawn Queue
USTRUCT()
struct FSpawnQueueItem
{
	GENERATED_BODY()

	enum ESpawnType
	{
		Enemy,
		Item,
		Chest
	};

	ESpawnType Type;
	FTransform Transform;
	TSubclassOf<AA1CreatureBase> EnemyClass;
	int32 ItemIndex;

	FSpawnQueueItem()
	{
		Type = Enemy;
		Transform = FTransform::Identity;
		EnemyClass = nullptr;
		ItemIndex = 0;
	}

	FSpawnQueueItem(ESpawnType InType, const FTransform& InTransform, TSubclassOf<AA1CreatureBase> InEnemyClass = nullptr, int32 InItemIndex = 0)
		: Type(InType), Transform(InTransform), EnemyClass(InEnemyClass), ItemIndex(InItemIndex) {
	}
};

/*
 * Raider Room with procedural mesh cliff creation
 */
UCLASS()
class AA1RaiderRoom : public AA1MasterRoom
{
	GENERATED_BODY()
public:
	AA1RaiderRoom(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaSeconds) override;


/********************************************
* Spawn Functions Section
********************************************/
	UFUNCTION(BlueprintCallable)
	void SpawnEnemies(TSubclassOf<AA1CreatureBase> CreatureClass);

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy(FTransform SpawnTransform, TSubclassOf<AA1CreatureBase> EnemyClass);

	UFUNCTION(BlueprintCallable)
	void SpawnItems();

	UFUNCTION(BlueprintCallable)
	void SpawnItem(int32 idx);

	UFUNCTION(BlueprintCallable)
	void SpawnChest(FTransform SpawnTransform);

	UFUNCTION(BlueprintCallable)
	void RemoveEnemy();

	UFUNCTION(BlueprintCallable)
	void RemoveItem();

	UFUNCTION(BlueprintCallable)
	void RemoveChest();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnPlundererSpawner();

	// Room Type Helper Function
	void SetRoomType(int32 RoomIndex);
	FORCEINLINE ERoomType GetRoomType() const { return RoomType; }
	FORCEINLINE bool GetCanMakeCliff() const { return bCanMakeCliff; }


/********************************************
 * Spawn Queue Function Section
 ********************************************/
private:
	// Spawn Queue Functions
	void AddEnemyToQueue(TSubclassOf<AA1CreatureBase> EnemyClass, const FTransform& Transform);
	void AddItemToQueue(int32 ItemIndex);
	void AddChestToQueue(const FTransform& Transform);
	void ProcessSpawnQueue();

/********************************************
* Cliff Function Section
********************************************/
public:
	// Cliff Creation
	void CreateCliffHole();

private:
	// Cliff Creation Sequence Functions
	void StartCliffCreation();
	void ExecuteCliffStep();
	void Step01_GetFloorDimensions();
	void Step02_CreateBasePlane();
	void Step03_CreateHoleGeometry();
	void Step04_RemoveTrianglesInHole();
	void Step05_GenerateCliffWalls();
	void Step06_CreateFinalMesh();
	void Step07_FinalizeCliff();
	void Step08_FinalizeCliff_2();

	// Cliff Helper Functions
	void LogStepTime(const FString& StepName);
	void StartStepTimer();
	FVector GetSquareEdgeIntersection(const FVector& P1, const FVector& P2, int32 EdgeIndex);
	TArray<FVector> ClipTriangleToSquare(const TArray<FVector>& TriangleVertices);
	bool IsVertexOutsideHole(const FVector& Vertex);
	bool IsVertexInSquareHole(const FVector& Vertex);

protected:

/********************************************
* Core Component
********************************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Components")
	TObjectPtr<UStaticMeshComponent> Inner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Components")
	TObjectPtr<UStaticMeshComponent> Floor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Components")
	TObjectPtr<UProceduralMeshComponent> CliffProceduralMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Raider|Materials")
	TArray<UMaterialInterface*> StaticMeshMaterials;


/********************************************
* Spawn Component
********************************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TObjectPtr<USceneComponent> EssentialSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TObjectPtr<USceneComponent> OptionalSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TObjectPtr<USceneComponent> ItemSpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TObjectPtr<USceneComponent> RemoveItemPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TObjectPtr<USceneComponent> ItemBoxSpawnLocation;


/********************************************
* Replicated Arrays
********************************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TArray<TObjectPtr<USceneComponent>> ItemSpawnLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Spawn")
	TArray<TObjectPtr<USceneComponent>> ItemBoxSpawnLocations;

	UPROPERTY(VisibleAnywhere, Category = "Raider|Spawned", Replicated)
	TArray<TObjectPtr<AA1CreatureBase>> SpawnedCreatures;

	UPROPERTY(VisibleAnywhere, Category = "Raider|Spawned", Replicated)
	TArray<TObjectPtr<AA1EquipmentBase>> SpawnedItems;

	UPROPERTY(VisibleAnywhere, Category = "Raider|Spawned", Replicated)
	TArray<TObjectPtr<AA1ChestBase>> SpawnedChests;


/********************************************
* Class & Template
********************************************/
	UPROPERTY(EditDefaultsOnly, Category = "Raider|Classes")
	TSubclassOf<AA1ChestBase> ChestClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Items")
	TArray<TSubclassOf<class UA1ItemTemplate>> CachedItemTemplates;


/********************************************
* Spawn Setting
********************************************/
	UPROPERTY(EditDefaultsOnly, Category = "Raider|Settings")
	int32 MinOptionalMonster;

	UPROPERTY(EditDefaultsOnly, Category = "Raider|Settings")
	int32 MaxOptionalMonster;

	UPROPERTY(EditDefaultsOnly, Category = "Raider|Settings")
	int32 MinOptionalItemCount;

	UPROPERTY(EditDefaultsOnly, Category = "Raider|Settings")
	int32 MaxOptionalItemCount;

	UPROPERTY(EditDefaultsOnly, Category = "Raider|Settings")
	int32 SpawnPercentage;


/********************************************
* Room Type and Set
********************************************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raider|Type")
	ERoomType RoomType = ERoomType::Rounge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Raider|Cliff")
	bool bCanMakeCliff = false;

/********************************************
* Cliff Creation Section
********************************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Cliff", meta = (ClampMin = "200", ClampMax = "600"))
	float HoleSize = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Cliff", meta = (ClampMin = "100", ClampMax = "400"))
	float HoleDepth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Performance", meta = (ClampMin = "0.01", ClampMax = "0.1"))
	float StepTime = 0.016f; // 16ms per step

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raider|Debug")
	bool bShowStepTiming = true; // Show individual step timing

	UPROPERTY()
	TArray<int32> HoleBoundaryIndices;

private:
/********************************************
* Spawn Queue & Timing
********************************************/
	TQueue<FSpawnQueueItem> SpawnQueue;

	UPROPERTY(EditAnywhere, Category = "Raider|Performance")
	float SpawnInterval = 0.3f;
	float SpawnTimer = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Raider|Performance")
	int32 SpawnPerTick = 1;

/********************************************
 * Cliff Section
 ********************************************/

// Random Stream
	FRandomStream RandomStream;

//Cliff Creation
	FTimerHandle CliffTimerHandle;
	int32 CurrentCliffStep = 0;
	bool bCliffInProgress = false;
	double CliffStartTime = 0.0;
	double StepStartTime = 0.0;

// Procedural Mesh Data Storage
	TArray<FVector> OriginalVertices;
	TArray<int32> OriginalTriangles;
	TArray<FVector> OriginalNormals;
	TArray<FVector2D> OriginalUVs;

	TArray<FVector> FinalVertices;
	TArray<int32> FinalTriangles;
	TArray<FVector> FinalNormals;
	TArray<FVector2D> FinalUVs;

	FTransform OriginalFloorTransform;

// Hole Geometry
	FVector HoleCenter;
	float HoleHalfSize;
	float FloorSizeX;
	float FloorSizeY;
};