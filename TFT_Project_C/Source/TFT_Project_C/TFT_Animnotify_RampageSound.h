// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "TFT_Animnotify_RampageSound.generated.h"

/**
 * 
 */
UCLASS()
class TFT_PROJECT_C_API UTFT_Animnotify_RampageSound : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FString SoundKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FVector SoundLocationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FRotator SoundRotationOffset;
};
