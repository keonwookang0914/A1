// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


/**
 * when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list
 **/

#define A1_ObjectChannel_Weapon         ECC_GameTraceChannel6
#define A1_ObjectChannel_Projectile		ECC_GameTraceChannel7
#define A1_TraceChannel_AimAssist       ECC_GameTraceChannel1
#define A1_TraceChannel_Interaction		ECC_GameTraceChannel8
#define A1_TraceChannel_Raider			ECC_GameTraceChannel9

#define A1_PhysicalMaterial_Default		SurfaceType_Default
#define A1_PhysicalMaterial_Character	SurfaceType1
#define A1_PhysicalMaterial_Rock		SurfaceType2
#define A1_PhysicalMaterial_Wood		SurfaceType3
