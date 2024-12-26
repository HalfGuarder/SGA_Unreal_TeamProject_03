// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TFT_GameStartWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartPageEvent);

UCLASS()
class TFT_PROJECT_C_API UTFT_GameStartWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION()
	void StartEvent();
	UFUNCTION()
	void ExitEvent();

	UFUNCTION()
	void ShowLoadingScreen();


	UPROPERTY(BlueprintAssignable, Category = "Event")
	FStartPageEvent _StartEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LoadingScreen, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> _loadingScreen;


private:
	void HideUI();
};
