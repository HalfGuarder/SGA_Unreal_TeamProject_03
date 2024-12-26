// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_MeshComponent.h"

#include "TFT_Creature.h"

UTFT_MeshComponent::UTFT_MeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UTFT_MeshComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UTFT_MeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UTFT_MeshComponent::SetMesh(FString path)
{
	auto character = Cast<ATFT_Creature>(GetOwner());

	ConstructorHelpers::FObjectFinder<USkeletalMesh> skm
	(*path);
	if (skm.Succeeded())
	{
		character->GetMesh()->SetSkeletalMesh(skm.Object);
	}

	character->GetMesh()->SetRelativeLocationAndRotation
	(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
}

void UTFT_MeshComponent::ChangeMesh(TObjectPtr<USkeletalMesh> mesh)
{
	auto character = Cast<ATFT_Creature>(GetOwner());

	character->GetMesh()->SetSkeletalMesh(mesh);
	// character->GetMesh()->SetSkeletalMeshAsset(mesh);

	character->GetMesh()->SetRelativeLocationAndRotation
	(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
}

