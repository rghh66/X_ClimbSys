// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GM_ClimbLadder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGM_ClimbLadder, All, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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
	
private:
	UFUNCTION()
	void OnClimbLadderFinishNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	void OverClimb(UAnimMontage* Montage, bool bInterrupted, ACharacter* Executer);

public:
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder")
	bool bCanClimb;

	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimb", EditConditionHides))
	TObjectPtr<UAnimMontage> ClimbLadderMontage;
	
	UPROPERTY(EditAnywhere, Category = "GM|Climb Ladder", meta = (EditCondition = "bCanClimb", EditConditionHides))
	TObjectPtr<UAnimMontage> ClimbLadderToTopMontage;

private:
	bool bIsClimbing;

	bool bIsPlayClimbMontage;

	bool bIsPlayClimbToTopMontage;
};
