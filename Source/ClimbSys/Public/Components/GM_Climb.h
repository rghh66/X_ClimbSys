// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GM_Climb.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGM_Climb, All, All);

USTRUCT(BlueprintType)
struct FClimbSettings 
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere)
	FVector Offset;
};

UCLASS( ClassGroup=(GM), meta=(BlueprintSpawnableComponent) )
class CLIMBSYS_API UGM_Climb : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGM_Climb();

	UFUNCTION(BlueprintCallable, Category = "GM|Climb")
	void Climb();

protected:
	bool DetectClimbableSurface(FHitResult& OutForwardHit, FHitResult& OutDownwardHit) const;

	void StartClimbMovement(const FHitResult& ForwardHit, const FHitResult& DownwardHit);

private:
	void OnMontagePlayEnd(UAnimMontage* Montage, bool bInterrupted, ACharacter* Executer);

public:
	UPROPERTY(EditAnywhere, Category = "GM|Climb")
	bool bCanClimb;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	FClimbSettings HighClimbSettings;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	FClimbSettings LowClimbSettings;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	bool bUseWallNormalForRotation;

	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	float ForwardTraceDistance;

	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	float DownwardTraceHeight;

	UPROPERTY(EditAnywhere, Category = "GM|Climb", meta = (EditCondition = "bCanClimb", EditConditionHides))
	float DownwardTraceDepth;

private:
	bool bIsClimbing;
};
