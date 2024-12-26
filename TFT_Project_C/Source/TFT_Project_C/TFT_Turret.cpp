// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_Turret.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"

#include "TFT_Monster.h"
#include "TFT_Projectile.h"
#include "TFT_StatComponent.h"

ATFT_Turret::ATFT_Turret()
{
	PrimaryActorTick.bCanEverTick = true;

	_statCom = CreateDefaultSubobject<UTFT_StatComponent>(TEXT("Stat_Com"));

	_btmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomMesh"));

	_midMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MiddleMesh"));

	_topMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopMesh"));

	RootComponent = _btmMesh;
	_midMesh->SetupAttachment(_btmMesh);
	_topMesh->SetupAttachment(_midMesh);

	/*static ConstructorHelpers::FClassFinder<ATFT_Projectile> pc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Projectile_BP.TFT_Projectile_BP_C'"));
	if (pc.Succeeded())
	{
		_projectileClass = pc.Class;
	}*/
}

void ATFT_Turret::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(_destroyTimerHandle, this, &ATFT_Turret::DeleteSelf, 8.0f, false);
}

void ATFT_Turret::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (_statCom->IsValidLowLevel())
	{
		_statCom->SetLevelAndInit(1);
	}
}

void ATFT_Turret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	timer += DeltaTime;

	if (!bIsPreview && !bIsSpawned && timer >= 0.05f)
	{
		Attract();
		bIsSpawned = true;
	}

	if (!bIsPreview && timer >= 0.5f)
	{
		FindTarget();

		if (timer >= 1.0f)
		{
			Fire();
			timer = 0;
		}
	}
}

float ATFT_Turret::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	float damaged = -(_statCom->AddCurHp(-Damage));

	return damaged;
}

void ATFT_Turret::Attract()
{
	if (bIsPreview) return;

	auto world = GetWorld();
	FVector center = this->GetActorLocation();
	float searchRadius = 5000.0f;

	if (world == nullptr) return;
	TArray<FOverlapResult> overlapResult;
	FCollisionQueryParams qParams(NAME_None, false, this);

	bool bResult = world->OverlapMultiByChannel
	(
		overlapResult,
		center,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel9,
		FCollisionShape::MakeSphere(searchRadius),
		qParams
	);

	FColor drawColor = FColor::Green;

	if (bResult)
	{
		for (auto& result : overlapResult)
		{
			ATFT_Monster* enemy = Cast<ATFT_Monster>(result.GetActor());
			if (enemy != nullptr)
			{
				auto param = Cast<AActor>(this);
				enemy->ReStartBT(param);
			}
		}
	}
}

void ATFT_Turret::FindTarget()
{
	auto world = GetWorld();
	FVector center = this->GetActorLocation();
	float searchRadius = 1500.0f;

	if (world == nullptr) return;
	TArray<FOverlapResult> overlapResult;
	FCollisionQueryParams qParams(NAME_None, false, this);

	bool bResult = world->OverlapMultiByChannel
	(
		overlapResult,
		center,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel9,
		FCollisionShape::MakeSphere(searchRadius),
		qParams
	);

	FColor drawColor = FColor::Green;

	if (bResult)
	{
		for (auto& result : overlapResult)
		{
			if (_target != nullptr)
			{
				ATFT_Monster* enemy = Cast<ATFT_Monster>(result.GetActor());

				if (enemy == _target)
				{
					Aiming();

					drawColor = FColor::Red;

					// DrawDebugSphere(world, center, searchRadius, 20, drawColor, 1.0f);

					return;
				}
			}
		}

		for (auto& result : overlapResult)
		{
			ATFT_Monster* enemy = Cast<ATFT_Monster>(result.GetActor());

			if (enemy != nullptr)
			{
				_target = enemy;

				return;
			}
		}
		_target = nullptr;
	}

	_target = nullptr;

	// DrawDebugSphere(world, center, searchRadius, 20, drawColor, 1.0f);	
}

void ATFT_Turret::Aiming()
{
	FVector targetLoc = _target->GetActorLocation();

	FRotator midRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), targetLoc);
	FRotator topRot = UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), targetLoc);

	_midMesh->SetRelativeRotation(FRotator(0.0f, midRot.Yaw, 0.0f));
	_topMesh->SetRelativeRotation(FRotator(topRot.Pitch, 0.0f, 0.0f));
}

void ATFT_Turret::Fire()
{
	if (_projectileClass && _target != nullptr)
	{
		FVector start = _topMesh->GetForwardVector();
		FVector end = FRotator(_topMesh->GetRelativeRotation().Pitch - 5.0f, _midMesh->GetRelativeRotation().Yaw, 0.0f).Vector() + start;

		FVector direction = end - start;

		FVector fireLocation = _topMesh->GetComponentLocation() + (_topMesh->GetForwardVector() * 50.0f);
		FRotator fireRotation = FRotator(_topMesh->GetRelativeRotation().Pitch - 5.0f, _midMesh->GetRelativeRotation().Yaw, 0.0f);

		auto bullet = GetWorld()->SpawnActor<ATFT_Projectile>(_projectileClass, fireLocation, fireRotation);
		bullet->FireInDirection(direction, 50.0f);
	}
}

void ATFT_Turret::DeleteSelf()
{
	this->Destroy();
}

