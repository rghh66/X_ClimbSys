// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GM_ClimbLadder.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogGM_ClimbLadder)

UGM_ClimbLadder::UGM_ClimbLadder()
{
	PrimaryComponentTick.bCanEverTick = true;

	bCanClimb = false;
	bIsClimbing = false;
	bIsPlayClimbMontage = false;
	bIsPlayClimbToTopMontage = false;
}

void UGM_ClimbLadder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsClimbing || bIsPlayClimbMontage || bIsPlayClimbToTopMontage)
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
}

void UGM_ClimbLadder::ClimbLadder()
{
	if (!bCanClimb)
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

	UCharacterMovementComponent* Movement = Executer->GetCharacterMovement();
	UAnimInstance* AnimInstance = Executer->GetMesh() ? Executer->GetMesh()->GetAnimInstance() : nullptr;
	if (IsValid(Movement) && IsValid(AnimInstance))
	{
		Movement->SetMovementMode(EMovementMode::MOVE_Flying);
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UGM_ClimbLadder::OnClimbLadderFinishNotify);
		bIsClimbing = true;
	}
}

void UGM_ClimbLadder::ClimbToTop()
{
	if (!bCanClimb || !bIsClimbing)
	{
		return;
	}

	bIsPlayClimbMontage = false;

	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	check(IsValid(Executer));

	if (UAnimInstance* AnimInstance = Executer->GetMesh()->GetAnimInstance(); IsValid(AnimInstance))
	{
		bIsPlayClimbToTopMontage = true;
		AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UGM_ClimbLadder::OnClimbLadderFinishNotify);
		FOnMontageEnded OnToTopEnded;
		OnToTopEnded.BindUObject(this, &UGM_ClimbLadder::OverClimb, Executer);
		AnimInstance->Montage_Play(ClimbLadderToTopMontage);
		AnimInstance->Montage_SetEndDelegate(OnToTopEnded, ClimbLadderToTopMontage);
	}
	else
	{
		check(false);
	}
}

void UGM_ClimbLadder::OverClimb(UAnimMontage* Montage, bool bInterrupted, ACharacter* Executer)
{
	if (!bCanClimb || !bIsClimbing)
	{
		return;
	}

	bIsPlayClimbToTopMontage = false;
	bIsClimbing = false;

	if (Executer && Executer->GetCharacterMovement())
	{
		Executer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}
