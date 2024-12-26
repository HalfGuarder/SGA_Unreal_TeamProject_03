// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TFT_AnimInstance_BJ.generated.h"

DECLARE_MULTICAST_DELEGATE(AttackStartDelegate);
DECLARE_MULTICAST_DELEGATE(AttackHitDelegate);
DECLARE_MULTICAST_DELEGATE(DeathStartDelegate);
DECLARE_MULTICAST_DELEGATE(DeathEndDelegate);
DECLARE_MULTICAST_DELEGATE(BossDeathEndDelegate);

UCLASS()
class TFT_PROJECT_C_API UTFT_AnimInstance_BJ : public UAnimInstance
{
	GENERATED_BODY()

public:
	UTFT_AnimInstance_BJ();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void PlayAttackMontage();
	void PlaySkillMontage();
	void PlaySlashMontage();
	void PlayDeathMontage();

	void JumpToSection(int32 sectionIndex);

	UFUNCTION()
	void AnimNotify_AttackStart();

	UFUNCTION()
	void AnimNotify_AttackHit();

	UFUNCTION()
	void AnimNotify_DeathStart();

	UFUNCTION()
	void AnimNotify_DeathEnd();


	AttackStartDelegate _attackStartDelegate;
	AttackHitDelegate _attackHitDelegate;
	DeathStartDelegate _deathStartDelegate;
	DeathEndDelegate _deathEndDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Meta = (AllowPrivateAccess = true))
	class UAnimMontage* _attackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Meta = (AllowPrivateAccess = true))
	UAnimMontage* _skillMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Meta = (AllowPrivateAccess = true))
	UAnimMontage* _slashMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn, Meta = (AllowPrivateAccess = true))
	UAnimMontage* _deathMontage;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float _speed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool _isfalling;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool _isdead;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float _vertical;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float _horizontal;
};
