// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_PlayerSkillComponent.h"

#include "TFT_Projectile.h"
#include "TFT_Creature.h"

#include "Engine/OverlapResult.h"
#include "Engine/DamageEvents.h"

UTFT_PlayerSkillComponent::UTFT_PlayerSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<ATFT_Projectile> pc
	(TEXT("/Script/Engine.Blueprint'/Game/Blueprints/Characters/Player/Weapons/TFT_Projectile_BP.TFT_Projectile_BP_C'"));
	if (pc.Succeeded())
	{
		_bulletClass = pc.Class;
	}

}

void UTFT_PlayerSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateBullet(_bulletClass, _bulletArray, 20);

	_bulletPenetrationRate = 50.0f;
}

void UTFT_PlayerSkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UTFT_PlayerSkillComponent::Fire(FVector start, FRotator rot, FVector fireDir)
{
	if (!_bulletArray.IsEmpty())
	{
		for (auto bullet : _bulletArray)
		{
			if (bullet->bIsFired) continue;

			bullet->Active();

			bullet->SetActorLocationAndRotation(start, rot);

			bullet->FireInDirection(fireDir, _bulletPenetrationRate);

			break;
		}
	}
}

void UTFT_PlayerSkillComponent::AttackHit(float damage, AController* controller)
{
	// ===================== �ʱ� ���� =====================
	FCollisionQueryParams params(NAME_None, false, GetOwner());
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	FVector PlayerForwardVector = GetOwner()->GetActorForwardVector();
	float SwordAttackRange = 300.0f;   // �ݿ��� ������ (���ݹ���)
	int32 NumSegments = 16;            // �ݿ� ���׸�Ʈ ��
	float AngleStep = 180.0f / NumSegments; // ���� ���� (180��: �ݿ�)

	// ===================== ���� �ݿ� ���� ���� ���� =====================
	TArray<FHitResult> HitResults;
	bool bResult = GetWorld()->SweepMultiByChannel(
		HitResults,
		PlayerLocation,                                    // ������
		PlayerLocation,                                    // ���� (�� ���·� Sweep)
		FQuat::Identity,                                   // ȸ�� ����
		ECollisionChannel::ECC_GameTraceChannel7,          // ä��
		FCollisionShape::MakeSphere(SwordAttackRange),     // �ݰ游ŭ Sphere ���
		params
	);

	// 
	FColor LineColor = bResult && !HitResults.IsEmpty() ? FColor::Red : FColor::Green;
	float DebugDuration = 2.0f;

	// ���� �ݿ� �� ���͸� �� Ÿ�� ó��
	if (bResult && !HitResults.IsEmpty())
	{
		LineColor = FColor::Red; // Ÿ�� ���� -> ������
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor->IsValidLowLevel())
			{
				// ���� 180�� �� Ÿ�� ���͸�
				FVector ToTarget = (HitActor->GetActorLocation() - PlayerLocation).GetSafeNormal();
				float DotProduct = FVector::DotProduct(PlayerForwardVector, ToTarget);
				float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

				if (AngleInDegrees <= 90.0f) // ���� �ݿ� ���� ��
				{
					// Ÿ�� ó��
					FDamageEvent DamageEvent;
					HitActor->TakeDamage(damage, DamageEvent, controller, GetOwner());

					UE_LOG(LogTemp, Log, TEXT("%f"), damage);

					// Ÿ�ݵ� ��ġ�� �� ���
					// DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Red, false, DebugDuration);
				}
			}
		}
	}

	// ===================== �������� ���� �ݿ� �׸��� =====================

	for (int32 i = 0; i < NumSegments; ++i)
	{
		// ���� ���
		float Angle1 = -90.0f + i * AngleStep;           // ���� ����
		float Angle2 = -90.0f + (i + 1) * AngleStep;     // ���� ����

		// ������ ���� ���� ������ �������� ��ǥ ���
		FVector Point1 = PlayerLocation + PlayerForwardVector.RotateAngleAxis(Angle1, FVector::UpVector) * SwordAttackRange;
		FVector Point2 = PlayerLocation + PlayerForwardVector.RotateAngleAxis(Angle2, FVector::UpVector) * SwordAttackRange;

		// �� ���� ���� ���� �׸���
		// DrawDebugLine(GetWorld(), Point1, Point2, LineColor, false, DebugDuration, 0, 1.5f);
	}



	// FHitResult hitResult;
	/*FCollisionQueryParams params(NAME_None, false, GetOwner());
	FVector playerLocation = GetOwner()->GetActorLocation();
	FVector playerFwdVector = GetOwner()->GetActorForwardVector();

	TArray<FHitResult> hitResults;

	bool bResult = GetWorld()->SweepMultiByChannel
	(
		hitResults,
		playerLocation,
		playerLocation + playerFwdVector * _swordAttackRange * 0.5f,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeCapsule(_swordAttackRadius, _swordAttackRange * 0.5f),
		params
	);

	FVector vec = playerFwdVector * _swordAttackRange;
	FVector center = playerLocation + vec;
	FColor drawColor = FColor::Green;

	if (bResult && !hitResults.IsEmpty())
	{
		for (int32 i = 0; i < hitResults.Num(); i++)
		{
			if (hitResults[i].GetActor()->IsValidLowLevel())
			{
				drawColor = FColor::Red;
				FDamageEvent damageEvent;

				hitResults[i].GetActor()->TakeDamage(damage, damageEvent, controller, GetOwner());
				FVector hitPoint = hitResults[i].ImpactPoint;
			}
		}
	}
	DrawDebugCapsule(GetWorld(), center, _swordAttackRange * 0.5f, _swordAttackRadius, FQuat::Identity, drawColor, false, 2.0f);
	*/

	/*bool bResult = GetWorld()->SweepSingleByChannel
	(
		hitResult,
		playerLocation,
		playerLocation + playerFwdVector * _swordAttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(_swordAttackRadius),
		params
	);*/


	/*if (bResult && hitResult.GetActor()->IsValidLowLevel())
	{
		drawColor = FColor::Red;
		FDamageEvent damageEvent;

		float actualDamage = hitResult.GetActor()->TakeDamage(damage, damageEvent, controller, GetOwner());
		FVector hitPoint = hitResult.ImpactPoint;

		if (actualDamage > 0)
		{
			ATFT_Creature* target = Cast<ATFT_Creature>(hitResult.GetActor());
			if (target != nullptr && !target->bIsOnState)
			{
				switch (_curAttackIndex)
				{
				case 1:
					target->SetState(StateType::Airborne);
					break;
				case 2:
					target->SetState(StateType::Stun);
					break;
				default:
					break;
				}
			}
		}
	}*/

	//DrawDebugSphere(GetWorld(), center, _swordAttackRadius, 20, drawColor, false, 2.0f);
}

void UTFT_PlayerSkillComponent::Q_SkillHit(float damage, AController* controller)
{
	FCollisionQueryParams params(NAME_None, false, GetOwner());
	FVector playerLocation = GetOwner()->GetActorLocation();
	FVector playerFwdVector = GetOwner()->GetActorForwardVector();

	TArray<FHitResult> hitResults;

	bool bResult = GetWorld()->SweepMultiByChannel
	(
		hitResults,
		playerLocation,
		playerLocation + playerFwdVector * _swordAttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(_swordAttackRadius),
		params
	);

	FVector vec = playerFwdVector * _swordAttackRange;
	FVector center = playerLocation + vec;
	FColor drawColor = FColor::Green;

	if (bResult && !hitResults.IsEmpty())
	{
		for (int32 i = 0; i < hitResults.Num(); i++)
		{
			if (hitResults[i].GetActor()->IsValidLowLevel())
			{
				drawColor = FColor::Red;
				FDamageEvent damageEvent;

				float actualDamage = hitResults[i].GetActor()->TakeDamage(damage, damageEvent, controller, GetOwner());
				_hitPoint = hitResults[i].ImpactPoint;

				/*if (actualDamage > 0)
				{
					ATFT_Creature* target = Cast<ATFT_Creature>(hitResults[i].GetActor());
					if (target != nullptr && !target->bIsOnState)
					{
						target->SetState(StateType::Stun);
					}
				}*/
			}
		}
	}

	// DrawDebugSphere(GetWorld(), center, _swordAttackRadius, 20, drawColor, false, 2.0f);
}

void UTFT_PlayerSkillComponent::CreateBullet(TSubclassOf<ATFT_Projectile> subclass, TArray<ATFT_Projectile*>& array, int32 num)
{
	for (int32 i = 0; i < num; i++)
	{
		auto projectile = GetWorld()->SpawnActor(subclass);

		ATFT_Projectile* bullet = Cast<ATFT_Projectile>(projectile);

		array.Add(bullet);
		bullet->DeActive();
	}
}
