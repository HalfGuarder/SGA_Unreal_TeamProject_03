// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TFT_SoundManager.generated.h"

UCLASS()
class TFT_PROJECT_C_API ATFT_SoundManager : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ATFT_SoundManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateSoundCue(FString keyName, FString path);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void PlaySound(FString keyName, FVector playLocation, FRotator playRotator = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable)
	void FadeOutSound(FString keyName, float intensity);

private:
	int32 _poolCount = 5;
	TMap<FString, class USoundCue*> _table;

};
