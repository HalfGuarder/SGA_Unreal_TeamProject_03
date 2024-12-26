// Fill out your copyright notice in the Description page of Project Settings.


#include "TFT_AnimInstance_NormalRampage.h"
#include "TFT_NormalMonster_Rampage.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "TFT_Monster.h"

UTFT_AnimInstance_NormalRampage::UTFT_AnimInstance_NormalRampage()
{
    static ConstructorHelpers::FObjectFinder<UAnimMontage> am
    (TEXT("/Script/Engine.AnimMontage'/Game/Blueprints/Monster/BossMonster_DH/Animation/TFT_Rampage_Montager.TFT_Rampage_Montager'"));
    if (am.Succeeded())
    {
        _attackMontage = am.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> dth
    (TEXT("/Script/Engine.AnimMontage'/Game/Blueprints/Monster/BossMonster_Grux/Animation/TFT_Grux_Death_AnimMontage.TFT_Grux_Death_AnimMontage'"));
    if (dth.Succeeded())
    {
        _deathMontage = dth.Object;
    }
}

void UTFT_AnimInstance_NormalRampage::NativeUpdateAnimation(float DeltaSeconds)
{
    ATFT_Creature* myCharacter = Cast<ATFT_Creature>(TryGetPawnOwner());

    if (myCharacter != nullptr)
    {
        _speed = myCharacter->GetVelocity().Size();
        _isFalling = myCharacter->GetMovementComponent()->IsFalling();
        _vertical = _vertical + (myCharacter->_vertical - _vertical) * DeltaSeconds;
        _horizontal = _horizontal + (myCharacter->_horizontal - _horizontal) * DeltaSeconds;
        _isDead = (myCharacter->GetCurHp() <= 0);
    }
}

void UTFT_AnimInstance_NormalRampage::PlayAttackMontage()
{
    if (!Montage_IsPlaying(_attackMontage))
    {

        float playResult = Montage_Play(_attackMontage);


        if (playResult > 0.0f)
        {

        }
        else
        {

        }


        ATFT_Monster* myCharacter = Cast<ATFT_Monster>(TryGetPawnOwner());
        if (myCharacter)
        {

        }
        else
        {

        }
    }
}

void UTFT_AnimInstance_NormalRampage::PlayDeathMontage()
{
    if (!Montage_IsPlaying(_deathMontage))
    {
        Montage_Play(_deathMontage);
    }
}

void UTFT_AnimInstance_NormalRampage::JumpToSection(int32 sectionIndex)
{
    FName sectionName = FName(*FString::Printf(TEXT("Attack%d"), sectionIndex));
    Montage_JumpToSection(sectionName);
}

void UTFT_AnimInstance_NormalRampage::AnimNotify_AttackStart()
{
    _attackStartDelegate.Broadcast();
}

void UTFT_AnimInstance_NormalRampage::AnimNotify_AttackHit()
{
    _attackHitDelegate.Broadcast();
}

void UTFT_AnimInstance_NormalRampage::AnimNotify_DeathStart()
{
    _deathStartDelegate.Broadcast();
}

void UTFT_AnimInstance_NormalRampage::AnimNotify_DeathEnd()
{
    _deathEndDelegate.Broadcast();
}
