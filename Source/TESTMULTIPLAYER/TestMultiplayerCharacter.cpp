// Fill out your copyright notice in the Description page of Project Settings.


#include "TestMultiplayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/InputSettings.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "TM_AttributeSetBase.h"
#include "DataAssets/CharacterDataAsset.h"
#include "TM_AbilitySystemComponentBase.h"

#include "Net/UnrealNetwork.h"

#include "ActorComponents/TM_CharacterMovementComponent.h"

ATestMultiplayerCharacter::ATestMultiplayerCharacter(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer.SetDefaultSubobjectClass<UTM_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    TurnRateGamepad = 50.f;


    //AbilitySystem


    AbillitySystemComponent = CreateDefaultSubobject<UTM_AbilitySystemComponentBase>(TEXT("AbilitySystemComponent"));
    AbillitySystemComponent->SetIsReplicated(true);
    AbillitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    AttributesSet = CreateDefaultSubobject<UTM_AttributeSetBase>(TEXT("AttributesSet"));
}

void ATestMultiplayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if(IsValid(CharacterDataAsset))
    {
        SetCharacterData(CharacterDataAsset->CharacterData);
    }
}


bool ATestMultiplayerCharacter::ApplayGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{
    if (!Effect.Get()) return false;

	FGameplayEffectSpecHandle SpecHandle = AbillitySystemComponent->MakeOutgoingSpec(Effect, 1.f, InEffectContext);
	if (SpecHandle.IsValid())
    {
		FActiveGameplayEffectHandle ActiveHandle = AbillitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

        return ActiveHandle.WasSuccessfullyApplied();
    }

    return false;
}


void ATestMultiplayerCharacter::GiveAbilities()
{
    if(HasAuthority() && AbillitySystemComponent)
    {
		for (auto DefaultAbility : CharacterData.Abilities)
		{
            AbillitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility));
		}
    }
}

void ATestMultiplayerCharacter::ApplayStartupEffects()
{
    if(GetLocalRole() == ROLE_Authority)
    {
        FGameplayEffectContextHandle EffectContext = AbillitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(this);


        for(auto CharacterEffect : CharacterData.Effects)
        {
            ApplayGameplayEffectToSelf(CharacterEffect, EffectContext);
        }
    }
}

void ATestMultiplayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

	AbillitySystemComponent->InitAbilityActorInfo(this, this);

    GiveAbilities();
    ApplayStartupEffects();
}

void ATestMultiplayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    AbillitySystemComponent->InitAbilityActorInfo(this, this);
}

void ATestMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ATestMultiplayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("Move Right / Left", this, &ATestMultiplayerCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);

    PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ATestMultiplayerCharacter::TurnAtRate);
    PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ATestMultiplayerCharacter::LookUpAtRate);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
}





void ATestMultiplayerCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void ATestMultiplayerCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void ATestMultiplayerCharacter::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATestMultiplayerCharacter::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

FCharacterData ATestMultiplayerCharacter::GetCharacterData() const
{
    return FCharacterData();
}

void ATestMultiplayerCharacter::SetCharacterData(const FCharacterData& InCharacterData)
{
    CharacterData = InCharacterData;

    InitFromCharacterData(CharacterData);
}


void ATestMultiplayerCharacter::InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication)
{

}


void ATestMultiplayerCharacter::OnRep_CharacterData()
{
    InitFromCharacterData(CharacterData);
}



void ATestMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATestMultiplayerCharacter, CharacterData);

}