// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFT_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TFT_PROJECT_C_API ATFT_PlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* _inputMappingContext;
};
