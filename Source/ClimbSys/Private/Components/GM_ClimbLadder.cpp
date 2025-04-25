// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GM_ClimbLadder.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogGM_ClimbLadder)

UGM_ClimbLadder::UGM_ClimbLadder()
{
	PrimaryComponentTick.bCanEverTick = true;

	bCanClimb = false;
	CurrentClimbState = EClimbState::None;
}

void UGM_ClimbLadder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bCanClimb || CurrentClimbState != EClimbState::Climbing || bIsPlayClimbMontage)
	{
		return;
	}

	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	if (IsValid(Executer) && IsValid(ClimbLadderMontage))
	{
		if (UAnimInstance* AnimInstance = Executer->GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
		{
			bIsPlayClimbMontage = true;
			AnimInstance->Montage_Play(ClimbLadderMontage);
		}
	}
}

void UGM_ClimbLadder::OnClimbLadderFinishNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	bIsPlayClimbMontage = false;
	UE_LOG(LogGM_ClimbLadder, Log, TEXT("ClimbLadderFinish"));
}

void UGM_ClimbLadder::ClimbLadder()
{
	if (!bCanClimb || CurrentClimbState != EClimbState::None)
	{
		return;
	}

	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	if (!IsValid(Executer))
	{
		UE_LOG(LogGM_ClimbLadder, Warning, TEXT("Climb component is not attached to a valid Character."));
		return;
	}

	if (!IsValid(ClimbLadderMontage))
	{
		UE_LOG(LogGM_ClimbLadder, Error, TEXT("Check Montage asset set for [%s]"), *Executer->GetFName().ToString());
		return;
	};

	UAnimInstance* AnimInstance = Executer->GetMesh() ? Executer->GetMesh()->GetAnimInstance() : nullptr;
	if (IsValid(AnimInstance))
	{
		Executer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UGM_ClimbLadder::OnClimbLadderFinishNotify);
		CurrentClimbState = EClimbState::Climbing; 
		CachedAnimInstance = AnimInstance;
	}
}

void UGM_ClimbLadder::ClimbToTop()
{
	if (!bCanClimb || CurrentClimbState != EClimbState::Climbing)
	{
		return;
	}

	if (CachedAnimInstance.IsValid())
	{
		CurrentClimbState = EClimbState::ClimbingToTop;
		CachedAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGM_ClimbLadder::OnClimbLadderFinishNotify);
		FOnMontageEnded OnToTopEnded;
		OnToTopEnded.BindUObject(this, &UGM_ClimbLadder::OverClimb);
		CachedAnimInstance->Montage_Play(ClimbLadderToTopMontage);
		CachedAnimInstance->Montage_SetEndDelegate(OnToTopEnded, ClimbLadderToTopMontage);
	}
}

void UGM_ClimbLadder::OverClimb(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bCanClimb || CurrentClimbState != EClimbState::ClimbingToTop)
	{
		return;
	}

	bIsPlayClimbMontage = false;

	ResetClimbState();
}

void UGM_ClimbLadder::ResetClimbState()
{
	CurrentClimbState = EClimbState::None;

	if (ACharacter* Executer = Cast<ACharacter>(GetOwner()); IsValid(Executer))
	{
		Executer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void UGM_ClimbLadder::BeginDestroy()
{
	Super::BeginDestroy();

	if (ACharacter* Executer = Cast<ACharacter>(GetOwner())) 
	{
		UAnimInstance* AnimInstance = Executer->GetMesh() ? Executer->GetMesh()->GetAnimInstance() : nullptr;
		if (IsValid(AnimInstance))
		{
			AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);
		}
	}
}
