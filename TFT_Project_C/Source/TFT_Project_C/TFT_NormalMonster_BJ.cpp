// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_NormalMonster_BJ.h"

#include "TFT_Player.h"
#include "TFT_AnimInstance_NormalBJ.h"
//#include "TFT_Widgets/TFT_HPBarWidget.h"
#include "TFT_SoundManager.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/SkeletalMeshComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "TFT_GameInstance.h"
//#include "TFT_EffectManager.h"

ATFT_NormalMonster_BJ::ATFT_NormalMonster_BJ()
{
    _possessionExp = 20;

}

void ATFT_NormalMonster_BJ::BeginPlay()
{
    Super::BeginPlay();

    if (GetMesh())
    {
        GetMesh()->SetAnimInstanceClass(UTFT_AnimInstance_NormalBJ::StaticClass());
    }


    GetCharacterMovement()->MaxWalkSpeed = 450.0f;
}

void ATFT_NormalMonster_BJ::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (_statCom->IsValidLowLevel())
    {
        _statCom->SetLevelAndInit(1);
        _statCom->_deathDelegate.AddUObject(this, &ATFT_NormalMonster_BJ::DeathStart);
    }
}

void ATFT_NormalMonster_BJ::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATFT_NormalMonster_BJ::SetMesh(FString path)
{
    _meshCom->SetMesh(path);
}

void ATFT_NormalMonster_BJ::AttackStart()
{
    Super::AttackStart();

    SOUNDMANAGER->PlaySound("Boss_Attack", GetActorLocation());
}

void ATFT_NormalMonster_BJ::AttackHit_Boss()
{
    TArray<FHitResult> hitResults;
    FCollisionQueryParams params(NAME_None, false, this);

    float attackRange = 500.0f;
    float attackRadius = 400.0f;

    bool bResult = GetWorld()->SweepMultiByChannel(
        hitResults,
        GetActorLocation(),
        GetActorLocation() + GetActorForwardVector() * attackRange,
        FQuat::Identity,
        ECollisionChannel::ECC_GameTraceChannel6,
        FCollisionShape::MakeSphere(attackRadius),
        params
    );

    FVector vec = GetActorForwardVector() * attackRange;
    FVector center = GetActorLocation() + vec * 0.5f;

    FColor drawColor = FColor::Green;

    if (bResult)
    {
        AActor* targetActor = nullptr;

        // ���͸�: "Player" �±׸� ���� ���͸� ó��
        for (const FHitResult& hitResult : hitResults)
        {
            AActor* hitActor = hitResult.GetActor();
            if (hitActor && hitActor->Tags.Contains(FName("Player")))
            {
                targetActor = hitActor; // "Player" �±׸� ���� ���� �߰�
                break; // ���� ����� "Player"�� Ÿ������ ó��
            }
        }

        if (targetActor)
        {
            // ������ ó��
            float hpRatio = _statCom->BossHPRatio();
            float damageMultiplier = (hpRatio < 0.3f) ? 2.0f : 1.0f;

            float baseDamage = _statCom->GetAttackDamage();
            float damage = baseDamage * damageMultiplier;

            FDamageEvent damageEvent;
            targetActor->TakeDamage(damage, damageEvent, GetController(), this);
            _hitPoint = targetActor->GetActorLocation();
            drawColor = FColor::Red;

            // ���� ȿ�� ����
            /*ATFT_Creature* targetCreature = Cast<ATFT_Creature>(targetActor);
            if (targetCreature != nullptr)
            {
                switch (_curAttackIndex)
                {
                case 1:
                    targetCreature->SetState(StateType::Airborne);
                    break;
                case 2:
                    targetCreature->SetState(StateType::Stun);
                    break;
                case 3:
                    targetCreature->SetState(StateType::Slow);
                    break;
                default:
                    break;
                }
            }*/
        }
    }

    // DrawDebugSphere(GetWorld(), center, attackRadius, 20, drawColor, false, 2.0f);
}

void ATFT_NormalMonster_BJ::Attack_AI()
{
    Super::Attack_AI();

    if (!_isAttacking && _animInstance_BJ != nullptr)
    {
        // ��ų �ִϸ��̼��� ��� ���� �ƴ� ��쿡�� ����
        if (!_animInstance_BJ->Montage_IsPlaying(_animInstance_BJ->_attackMontage))
        {
            // �Ϲ� ���� �ִϸ��̼� ����
            _animInstance_BJ->PlayAttackMontage();
            _isAttacking = true;

            // ���� ���� ���
            SOUNDMANAGER->PlaySound("Boss_Attack", GetActorLocation());

            // ���� �ε��� ������Ʈ
            _curAttackIndex %= 4;
            _curAttackIndex++;
            _animInstance_BJ->JumpToSection(_curAttackIndex);
        }
    }
}

void ATFT_NormalMonster_BJ::AttackEnd()
{
}

float ATFT_NormalMonster_BJ::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    return ActualDamage;
}

void ATFT_NormalMonster_BJ::DeathStart()
{
    Super::DeathStart();

    if (_animInstance_BJ)
    {
        _animInstance_BJ->PlayDeathMontage();
    }
    else
    {
        return;
    }



    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ATFT_NormalMonster_BJ::BossDisable, 2.0f, false);
}

void ATFT_NormalMonster_BJ::BossDisable()
{
    Super::DropItem(MonsterType::Normal);

    PrimaryActorTick.bCanEverTick = false;

    /*if (HpBarWidgetInstance)
    {
        HpBarWidgetInstance->RemoveFromParent();
        HpBarWidgetInstance = nullptr;
    }*/

    DeActive();
}

void ATFT_NormalMonster_BJ::SetAnimInstanceBind()
{
    Super::SetAnimInstanceBind();
    if (!bAnimBind)
    {
        _animInstance_BJ = Cast<UTFT_AnimInstance_NormalBJ>(GetMesh()->GetAnimInstance());

        if (_animInstance_BJ->IsValidLowLevel())
        {
            _animInstance_BJ->OnMontageEnded.AddDynamic(this, &ATFT_Creature::OnAttackEnded);
            _animInstance_BJ->_attackStartDelegate.AddUObject(this, &ATFT_NormalMonster_BJ::AttackStart);
            _animInstance_BJ->_attackHitDelegate.AddUObject(this, &ATFT_NormalMonster_BJ::AttackHit_Boss);
            _animInstance_BJ->_deathStartDelegate.AddUObject(this, &ATFT_NormalMonster_BJ::DeathStart);
            _animInstance_BJ->_deathEndDelegate.AddUObject(this, &ATFT_NormalMonster_BJ::BossDisable);

            bAnimBind = true;
        }
    }
}

void ATFT_NormalMonster_BJ::PreActive()
{
    Super::PreActive();

}

void ATFT_NormalMonster_BJ::Active()
{
    Super::Active();

}

void ATFT_NormalMonster_BJ::DeActive()
{
    Super::DeActive();

    if (_animInstance_BJ->IsValidLowLevel())
    {
        _animInstance_BJ->OnMontageEnded.Clear();
        _animInstance_BJ->_attackStartDelegate.Clear();
        _animInstance_BJ->_attackHitDelegate.Clear();
        _animInstance_BJ->_deathStartDelegate.Clear();
        _animInstance_BJ->_deathEndDelegate.Clear();
    }
}