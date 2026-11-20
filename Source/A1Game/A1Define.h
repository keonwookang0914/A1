#pragma once

UENUM(BlueprintType)
enum class ESlotState : uint8
{
	Default,
	Invalid,
	Valid
};

UENUM(BlueprintType)
enum class ERaiderType : uint8
{
	Despoiler,
	Plunderer,
	Barbarian,
	Explorer,
	Farmer,

	Count UMETA(Hidden)
};

/*---------------------------------------
*   Character가 가지고 있어야 할 정보
---------------------------------------*/

// Character의 skin 색상
UENUM(BlueprintType)
enum class ECharacterSkinType : uint8
{
	Black,
	Grey,
	White,
	Red,
	Yellow,
	Blue,

	Count UMETA(Hidden)
};

// Character Mesh Material type
UENUM(BlueprintType)
enum class EBodyType : uint8
{
	Head		UMETA(DisplayName = "Head"),
	Torso		UMETA(DisplayName = "Torso"),
	Limbs		UMETA(DisplayName = "Limbs"),

	Count UMETA(Hidden)
};

// 장비 장착 slot 타입 
UENUM(BlueprintType)
enum class EEquipmentSlotType : uint8
{
	LeftHand,
	RightHand,
	TwoHand,

	Count	UMETA(Hidden)
};

// 손 사용 여부
UENUM(BlueprintType)
enum class EEquipState : uint8
{
	Unarmed,
	Left,
	Right,
	Both,

	Count	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMainHandState : uint8
{
	Left,
	Right,

	Count	UMETA(Hidden)
};

/*---------------------------------------
*    Item이 가지고 있어야 할 정보
---------------------------------------*/
// Item 분류
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	Armor,
	Weapon,
	Utility,

	Count	UMETA(Hidden)
};

// 무기 종류
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed,
	OHS,
	THS,
	Gun,

	Count	UMETA(Hidden)
};

// Utillity 종류
UENUM(BlueprintType)
enum class EUtilityType : uint8
{
	Fuel,
	Food,
	Drink,
	Light,
	FoamGun UMETA(DisplayName = "FoamGun"),
	Documents,
	Repairkit,
	
	Count	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Poor,
	Common,
	Uncommon,
	Rare,
	Legendary,

	Count	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESpellType : uint8
{
	None,
	Projectile,
	AOE,

	Count	UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EOverlayTargetType : uint8
{
	None,
	Weapon,
	Character,
	All,
};


UENUM(BlueprintType)
enum class EA1TeamID : uint8
{
	NoTeam,
	Monster = 1,
};

UENUM(BlueprintType)
enum class EA1DeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};