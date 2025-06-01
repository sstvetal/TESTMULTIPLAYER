// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Abilities/GameplayAbility.h"
#include "MultiplayerTestGameTypes.h"
#include "TestMultiplayerCharacter.generated.h"

class UTM_AbilitySystemComponentBase;
class UTM_AttributeSetBase;

class UGameplayEffect;
class UGameplayAbility;

UCLASS()
class TESTMULTIPLAYER_API ATestMultiplayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATestMultiplayerCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void PostInitializeComponents() override;

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    class UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
    float TurnRateGamepad;


    bool ApplayGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext);

protected:
    void GiveAbilities();
    void ApplayStartupEffects();

	virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

	UPROPERTY(EditDefaultsOnly)
    UTM_AbilitySystemComponentBase* AbillitySystemComponent;

    UPROPERTY(Transient)
	UTM_AttributeSetBase* AttributesSet;



    void MoveForward(float Value);
    void MoveRight(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);


public:

    UFUNCTION(BlueprintCallable)
    FCharacterData GetCharacterData() const;

    UFUNCTION(BlueprintCallable)
    void SetCharacterData(const FCharacterData& InCharacterData);


protected:

    UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
    FCharacterData CharacterData;


    UFUNCTION()
    void OnRep_CharacterData();

    virtual void InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication = false);

    UPROPERTY(EditDefaultsOnly)
    class UCharacterDataAsset* CharacterDataAsset;


};

