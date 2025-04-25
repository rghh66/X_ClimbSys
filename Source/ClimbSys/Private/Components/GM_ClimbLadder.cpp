// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GM_ClimbLadder.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogGM_ClimbLadder)

UGM_ClimbLadder::UGM_ClimbLadder()
{
	PrimaryComponentTick.bCanEverTick = true;

	bCanClimbLadder = false;
	CurrentState = EClimbLadderState::None;
}

void UGM_ClimbLadder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGM_ClimbLadder::ClimbLadder()
{
	if (!bCanClimbLadder || CurrentState != EClimbLadderState::None)
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
		FOnMontageEnded OnClimbLadderEnded;
		OnClimbLadderEnded.BindUObject(this, &UGM_ClimbLadder::OnClimbLadderEnd);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UGM_ClimbLadder::OnClimbLadderNotify);
		AnimInstance->Montage_Play(ClimbLadderMontage);
		AnimInstance->Montage_SetEndDelegate(OnClimbLadderEnded, ClimbLadderMontage);
		CurrentState = EClimbLadderState::Climbing;
		CachedAnimInstance = AnimInstance;
	}
}

void UGM_ClimbLadder::OnClimbLadderNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == ClimbLadderNotifyName && CurrentState == EClimbLadderState::Climbing && CachedAnimInstance.IsValid())
	{
		CachedAnimInstance->Montage_JumpToSection(ClimbStartSection, ClimbLadderMontage);
		UE_LOG(LogGM_ClimbLadder, Log, TEXT("Jump to section [%s]"), *ClimbStartSection.ToString());
	}
}

void UGM_ClimbLadder::OnClimbLadderEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (CachedAnimInstance.IsValid())
	{
		CachedAnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGM_ClimbLadder::OnClimbLadderNotify);
	}
	if (bInterrupted && CurrentState == EClimbLadderState::Climbing)
	{
		ResetClimbState();
	}
}

void UGM_ClimbLadder::ClimbToTop()
{
	if (!bCanClimbLadder || CurrentState != EClimbLadderState::Climbing)
	{
		return;
	}

	if (CachedAnimInstance.IsValid())
	{
		CurrentState = EClimbLadderState::ClimbingToTop;
		FOnMontageEnded OnToTopEnded;
		OnToTopEnded.BindUObject(this, &UGM_ClimbLadder::OnClimbToTopEnd);
		CachedAnimInstance->Montage_Play(ClimbLadderToTopMontage);
		CachedAnimInstance->Montage_SetEndDelegate(OnToTopEnded, ClimbLadderToTopMontage);
	}
}

void UGM_ClimbLadder::OnClimbToTopEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bCanClimbLadder || CurrentState != EClimbLadderState::ClimbingToTop)
	{
		return;
	}

	ResetClimbState();
}

void UGM_ClimbLadder::ResetClimbState()
{
	CurrentState = EClimbLadderState::None;

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
