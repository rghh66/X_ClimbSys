// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GM_ClimbLadder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGM_ClimbLadder, All, All);

UENUM()
enum class EClimbLadderState : uint8
{
	None,
	Climbing,
	ClimbingToTop,
	Finished
};

UCLASS( ClassGroup=(GM), meta=(BlueprintSpawnableComponent) )
class CLIMBSYS_API UGM_ClimbLadder : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGM_ClimbLadder();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "GM|Climb Ladder")
	void ClimbLadder();
	
	UFUNCTION(BlueprintCallable, Category = "GM|Climb Ladder")
	void ClimbToTop();
	
protected:
	void BeginDestroy() override;

private:
	UFUNCTION()
	void OnClimbLadderNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	void OnClimbLadderEnd(UAnimMontage* Montage, bool bInterrupted);

	void OnClimbToTopEnd(UAnimMontage* Montage, bool bInterrupted);

	void ResetClimbState();

public:
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder")
	bool bCanClimbLadder;

	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimbLadder", EditConditionHides))
	TObjectPtr<UAnimMontage> ClimbLadderMontage;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimbLadder", EditConditionHides))
	FName ClimbLadderNotifyName;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimbLadder", EditConditionHides))
	FName ClimbStartSection;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimbLadder", EditConditionHides))
	TObjectPtr<UAnimMontage> ClimbLadderToTopMontage;

private:
	UPROPERTY(Transient)
	EClimbLadderState CurrentState;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAnimInstance> CachedAnimInstance;
};
