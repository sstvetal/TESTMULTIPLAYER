// Fill out your copyright notice in the Description page of Project Settings.


#include "TM_GameInstance.h"
#include "AbilitySystemGlobals.h"

void UTM_GameInstance::Init()
{
	Super::Init();

	UAbilitySystemGlobals::Get().InitGlobalData();
}
