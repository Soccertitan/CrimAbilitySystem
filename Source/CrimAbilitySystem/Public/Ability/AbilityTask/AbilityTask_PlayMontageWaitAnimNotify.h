// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PlayMontageWaitAnimNotify.generated.h"

class UCrimAbilitySystemComponent;

/** Signature type used, EventTag may be empty if it came from the montage callbacks. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayMontageAndWaitForNotifySignature, FGameplayTag, EventTag);

/**
 * This task combines PlayMontageAndWait and binds to the OnAnimNotifyDelegate. So you can wait for multiple types of activations such as from a melee combo.
 * Much of this code is copied from PlayMontageAndWait. We don't use WaitOnEvent as issues occur when both Client and
 * Server are trying to listen for the Gameplay Event callbacks.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UAbilityTask_PlayMontageWaitAnimNotify : public UAbilityTask
{
	GENERATED_BODY()
	
	public:
	UAbilityTask_PlayMontageWaitAnimNotify(const FObjectInitializer& ObjectInitializer);
	
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;
	virtual void OnDestroy(bool AbilityEnded) override;

	/** The montage completely finished playing */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForNotifySignature OnCompleted;

	/** The montage started blending out */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForNotifySignature OnBlendOut;

	/** The montage was interrupted */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForNotifySignature OnInterrupted;

	/** The ability task was explicitly cancelled by another ability */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForNotifySignature OnCancelled;

	/** One of the triggering gameplay events happened */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForNotifySignature NotifyReceived;

	/**
	 * Play a montage and wait for it end. If a notify event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with the tag.
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing.
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled.
	 *
	 * @param TaskInstanceName Set to override the name of this task, for later querying.
	 * @param MontageToPlay The montage to play on the character.
	 * @param EventTags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback.
	 * @param Rate Change to play the montage faster or slower.
	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageWaitAnimNotify* PlayMontageAndWaitForAnimNotify(
			UGameplayAbility* OwningAbility,
			FName TaskInstanceName,
			UAnimMontage* MontageToPlay,
			FGameplayTagContainer EventTags,
			float Rate = 1.f,
			FName StartSection = NAME_None,
			bool bStopWhenAbilityEnds = true,
			float AnimRootMotionTranslationScale = 1.f);

private:
	/** Montage that is playing */
	UPROPERTY()
	UAnimMontage* MontageToPlay;

	/** List of tags to match against gameplay events */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/** Playback rate */
	UPROPERTY()
	float Rate;

	/** Section to start montage from */
	UPROPERTY()
	FName StartSection;

	/** Modifies how root motion movement to apply */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** Rather montage should be aborted if ability ends */
	UPROPERTY()
	bool bStopWhenAbilityEnds;

	/** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
	bool StopPlayingMontage();

	/** Returns our ability system component */
	UCrimAbilitySystemComponent* GetAbilitySystemComponent() const;

	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
	void OnAbilityCancelled();
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnAnimNotify(const FGameplayTag& EventTag);

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
