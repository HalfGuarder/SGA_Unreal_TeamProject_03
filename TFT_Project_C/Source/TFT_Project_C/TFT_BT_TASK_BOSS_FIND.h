// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TFT_BT_TASK_BOSS_FIND.generated.h"

/**
 * 
 */
UCLASS()
class TFT_PROJECT_C_API UTFT_BT_TASK_BOSS_FIND : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UTFT_BT_TASK_BOSS_FIND();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)override;
};
