// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GM_Climb.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogGM_Climb)

UGM_Climb::UGM_Climb()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	bCanClimb = false;
	bIsClimbing = false;
	bUseWallNormalForRotation = false;

	ForwardTraceDistance = 100.f;
	DownwardTraceHeight = 200.f;
	DownwardTraceDepth = 300.f;
}

void UGM_Climb::Climb()
{
	if (!bCanClimb || bIsClimbing)
	{
		return;
	}

	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	if (!IsValid(Executer))
	{
		UE_LOG(LogGM_Climb, Warning, TEXT("Climb component is not attached to a valid Character."));
		return;
	}

	if (!IsValid(HighClimbSettings.Montage) || !IsValid(LowClimbSettings.Montage))
	{
		UE_LOG(LogGM_Climb, Error, TEXT("Check Montage asset set for [%s]"), *Executer->GetFName().ToString());
		return;
	}

	FHitResult ForwardHit, DownwardHit;
	if (!DetectClimbableSurface(ForwardHit, DownwardHit))
	{
		Executer->Jump();
		return;
	}

	StartClimbMovement(ForwardHit, DownwardHit);
}

bool UGM_Climb::DetectClimbableSurface(FHitResult& OutForwardHit, FHitResult& OutDownwardHit) const
{
	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	if (!IsValid(Executer))
	{
		return false;
	}

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(Executer);

	const FVector ForwardStart = Executer->GetActorLocation();
	const FVector ForwardEnd = ForwardStart + Executer->GetActorForwardVector() * ForwardTraceDistance;
	if (!GetWorld()->LineTraceSingleByChannel(
		OutForwardHit,
		ForwardStart,
		ForwardEnd,
		ECollisionChannel::ECC_Visibility,
		TraceParams))
	{
		return false;
	}

	// Downward
	const FVector DownwardStart = OutForwardHit.Location + FVector(0, 0, DownwardTraceHeight);
	const FVector DownwardEnd = DownwardStart + FVector(0, 0, -DownwardTraceDepth);
	return GetWorld()->SweepSingleByChannel(
		OutDownwardHit,
		DownwardStart,
		DownwardEnd,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(20.f),
		TraceParams);
}

void UGM_Climb::StartClimbMovement(const FHitResult& ForwardHit, const FHitResult& DownwardHit)
{
	ACharacter* Executer = Cast<ACharacter>(GetOwner());
	if (!IsValid(Executer)) return;

	bIsClimbing = true;

	if (UCharacterMovementComponent* Movement = Executer->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Flying);
	}

	const bool bIsHighClimb = (DownwardHit.Distance < 100.f);
	const FClimbSettings& Settings = bIsHighClimb ? HighClimbSettings : LowClimbSettings;

	const FVector WallNormal = ForwardHit.Normal;
	const FVector TargetLocation = DownwardHit.Location +
		FVector(Settings.Offset.X, 0.f, Settings.Offset.Z) +
		WallNormal * Settings.Offset.Y;

	Executer->SetActorLocation(TargetLocation);

	if (bUseWallNormalForRotation)
	{
		const FVector Forward = -WallNormal;
		const FVector Right = Executer->GetActorRightVector();
		const FVector Up = Executer->GetActorUpVector();

		const FRotator NewRotation = UKismetMathLibrary::MakeRotationFromAxes(Forward, Right, Up);
		Executer->SetActorRotation(NewRotation);
	}

	if (UAnimInstance* AnimInstance = Executer->GetMesh()->GetAnimInstance())
	{
		FOnMontageEnded OnMontageEnded;
		OnMontageEnded.BindUObject(this, &UGM_Climb::OnMontagePlayEnd, Executer);
		AnimInstance->Montage_Play(Settings.Montage);
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, Settings.Montage);
	}
}

void UGM_Climb::OnMontagePlayEnd(UAnimMontage* Montage, bool bInterrupted, ACharacter* Executer)
{
	bIsClimbing = false;

	if (IsValid(Executer) && IsValid(Executer->GetCharacterMovement()))
	{
		Executer->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}
