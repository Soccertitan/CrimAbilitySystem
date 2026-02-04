// Copyright Soccertitan 2025


#include "Ability/AbilityTask/AbilityTask_PlayMontageWaitAnimNotify.h"

#include "AbilitySystemGlobals.h"
#include "CrimAbilityLogChannels.h"
#include "CrimAbilitySystemComponent.h"
#include "GameFramework/Character.h"


UAbilityTask_PlayMontageWaitAnimNotify::UAbilityTask_PlayMontageWaitAnimNotify(const FObjectInitializer& ObjectInitializer)
{
	Rate = 1.f;
	bStopWhenAbilityEnds = true;
}

void UAbilityTask_PlayMontageWaitAnimNotify::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedMontage = false;

	if (UCrimAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// Bind to event callback
			EventHandle = ASC->OnAnimNotifyDelegate.AddUObject(this, &UAbilityTask_PlayMontageWaitAnimNotify::OnAnimNotify);

			if (ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f)
			{
				// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are pending kill.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayMontageWaitAnimNotify::OnAbilityCancelled);

				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayMontageWaitAnimNotify::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayMontageWaitAnimNotify::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				bPlayedMontage = true;
			}
		}
		else
		{
			UE_LOG(LogCrimAbilitySystem, Warning, TEXT("AbilityTask_PlayMontageAndWaitForNotify call to PlayMontage failed!"));
		}
	}
	else
	{
		UE_LOG(LogCrimAbilitySystem, Warning, TEXT("AbilityTask_PlayMontageAndWaitForNotify called on invalid AbilitySystemComponent"));
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogCrimAbilitySystem, Warning, TEXT("AbilityTask_PlayMontageAndWaitForNotify called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			//ABILITY_LOG(Display, TEXT("%s: OnCancelled"), *GetName());
			OnCancelled.Broadcast(FGameplayTag());
		}
	}

	SetWaitingOnAvatar();
}

void UAbilityTask_PlayMontageWaitAnimNotify::ExternalCancel()
{
	check(AbilitySystemComponent.Get());

	OnAbilityCancelled();

	Super::ExternalCancel();
}

FString UAbilityTask_PlayMontageWaitAnimNotify::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("AbilityTask_PlayMontageAndWaitForNotify. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void UAbilityTask_PlayMontageWaitAnimNotify::OnDestroy(bool AbilityEnded)
{
	// Note: Clearing montage end delegate isn't necessary since it is not a multicast and will be cleared when the next montage plays.
	// (If we are destroyed, it will detect this and not do anything)

	// This delegate, however, should be cleared as it is a multicast
	if (Ability)
	{
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	if (UCrimAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->OnAnimNotifyDelegate.Remove(EventHandle);
	}

	Super::OnDestroy(AbilityEnded);
}

UAbilityTask_PlayMontageWaitAnimNotify* UAbilityTask_PlayMontageWaitAnimNotify::PlayMontageAndWaitForAnimNotify(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay,
	FGameplayTagContainer EventTags, float Rate, FName StartSection, bool bStopWhenAbilityEnds,
	float AnimRootMotionTranslationScale)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	UAbilityTask_PlayMontageWaitAnimNotify* MyObj = NewAbilityTask<UAbilityTask_PlayMontageWaitAnimNotify>(OwningAbility, TaskInstanceName);
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

	return MyObj;
}

bool UAbilityTask_PlayMontageWaitAnimNotify::StopPlayingMontage()
{
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (ASC && Ability)
	{
		if (ASC->GetAnimatingAbility() == Ability
			&& ASC->GetCurrentMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			ASC->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

UCrimAbilitySystemComponent* UAbilityTask_PlayMontageWaitAnimNotify::GetAbilitySystemComponent() const
{
	return Cast<UCrimAbilitySystemComponent>(AbilitySystemComponent.Get());
}

void UAbilityTask_PlayMontageWaitAnimNotify::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			AbilitySystemComponent.Get()->ClearAnimatingAbility(Ability);

			// Reset AnimRootMotionTranslationScale
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy &&
				Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}

		}
	}

	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag());
		}
	}
}

void UAbilityTask_PlayMontageWaitAnimNotify::OnAbilityCancelled()
{
	if (StopPlayingMontage())
	{
		// Let the BP handle the interrupt as well
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag());
		}
	}
}

void UAbilityTask_PlayMontageWaitAnimNotify::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag());
		}
	}
	EndTask();
}

void UAbilityTask_PlayMontageWaitAnimNotify::OnAnimNotify(const FGameplayTag& EventTag)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (!EventTags.IsValid() || EventTag.MatchesAny(EventTags))
		{
			NotifyReceived.Broadcast(EventTag);
		}
	}
}