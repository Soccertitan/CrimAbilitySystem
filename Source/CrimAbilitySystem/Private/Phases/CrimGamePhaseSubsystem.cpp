// Copyright Soccertitan


#include "Phases/CrimGamePhaseSubsystem.h"

#include "CrimAbilitySystemComponent.h"
#include "Phases/GamePhaseLog.h"
#include "Phases/GamePhaseGameplayAbility.h"
#include "GameFramework/GameStateBase.h"

DEFINE_LOG_CATEGORY(LogGamePhase);

UGamePhaseSubsystem::UGamePhaseSubsystem()
{
}

void UGamePhaseSubsystem::StartPhase(TSubclassOf<UGamePhaseGameplayAbility> PhaseAbility, FCrimGamePhaseDelegate PhaseEndedCallback)
{
	UWorld* World = GetWorld();
	UCrimAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UCrimAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		FGameplayAbilitySpec PhaseSpec(PhaseAbility, 1, 0, this);
		FGameplayAbilitySpecHandle SpecHandle = GameState_ASC->GiveAbilityAndActivateOnce(PhaseSpec);
		FGameplayAbilitySpec* FoundSpec = GameState_ASC->FindAbilitySpecFromHandle(SpecHandle);
		
		if (FoundSpec && FoundSpec->IsActive())
		{
			FCrimGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(SpecHandle);
			Entry.PhaseEndedCallback = PhaseEndedCallback;
		}
		else
		{
			PhaseEndedCallback.ExecuteIfBound(nullptr);
		}
	}
}

void UGamePhaseSubsystem::WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCrimGamePhaseTagDelegate& WhenPhaseActive)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseActive;
	PhaseStartObservers.Add(Observer);

	if (IsPhaseActive(PhaseTag))
	{
		WhenPhaseActive.ExecuteIfBound(PhaseTag);
	}
}

void UGamePhaseSubsystem::WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCrimGamePhaseTagDelegate& WhenPhaseEnd)
{
	FPhaseObserver Observer;
	Observer.PhaseTag = PhaseTag;
	Observer.MatchType = MatchType;
	Observer.PhaseCallback = WhenPhaseEnd;
	PhaseEndObservers.Add(Observer);
}

bool UGamePhaseSubsystem::IsPhaseActive(const FGameplayTag& PhaseTag) const
{
	for (const auto& KVP : ActivePhaseMap)
	{
		const FCrimGamePhaseEntry& PhaseEntry = KVP.Value;
		if (PhaseEntry.PhaseTag.MatchesTag(PhaseTag))
		{
			return true;
		}
	}

	return false;
}

bool UGamePhaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UGamePhaseSubsystem::K2_StartPhase(TSubclassOf<UGamePhaseGameplayAbility> PhaseAbility, const FCrimGamePhaseDynamicDelegate& PhaseEnded)
{
	const FCrimGamePhaseDelegate EndedDelegate = FCrimGamePhaseDelegate::CreateWeakLambda(
		const_cast<UObject*>(PhaseEnded.GetUObject()), [PhaseEnded](const UGamePhaseGameplayAbility* PhaseAbility)
		{
			PhaseEnded.ExecuteIfBound(PhaseAbility);
		});

	StartPhase(PhaseAbility, EndedDelegate);
}

void UGamePhaseSubsystem::K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCrimGamePhaseTagDynamicDelegate WhenPhaseActive)
{
	const FCrimGamePhaseTagDelegate ActiveDelegate = FCrimGamePhaseTagDelegate::CreateWeakLambda(
		WhenPhaseActive.GetUObject(), [WhenPhaseActive](const FGameplayTag& PhaseTag)
		{
			WhenPhaseActive.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseStartsOrIsActive(PhaseTag, MatchType, ActiveDelegate);
}

void UGamePhaseSubsystem::K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCrimGamePhaseTagDynamicDelegate WhenPhaseEnd)
{
	const FCrimGamePhaseTagDelegate EndedDelegate = FCrimGamePhaseTagDelegate::CreateWeakLambda(
		WhenPhaseEnd.GetUObject(), [WhenPhaseEnd](const FGameplayTag& PhaseTag)
		{
			WhenPhaseEnd.ExecuteIfBound(PhaseTag);
		});

	WhenPhaseEnds(PhaseTag, MatchType, EndedDelegate);
}

void UGamePhaseSubsystem::OnBeginPhase(const UGamePhaseGameplayAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag IncomingPhaseTag = PhaseAbility->GetGamePhaseTag();

	UE_LOG(LogGamePhase, Log, TEXT("Beginning Phase '%s' (%s)"), *IncomingPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const UWorld* World = GetWorld();
	UCrimAbilitySystemComponent* GameState_ASC = World->GetGameState()->FindComponentByClass<UCrimAbilitySystemComponent>();
	if (ensure(GameState_ASC))
	{
		TArray<FGameplayAbilitySpec*> ActivePhases;
		for (const auto& KVP : ActivePhaseMap)
		{
			const FGameplayAbilitySpecHandle ActiveAbilityHandle = KVP.Key;
			if (FGameplayAbilitySpec* Spec = GameState_ASC->FindAbilitySpecFromHandle(ActiveAbilityHandle))
			{
				ActivePhases.Add(Spec);
			}
		}

		for (const FGameplayAbilitySpec* ActivePhase : ActivePhases)
		{
			const UGamePhaseGameplayAbility* ActivePhaseAbility = CastChecked<UGamePhaseGameplayAbility>(ActivePhase->Ability);
			const FGameplayTag ActivePhaseTag = ActivePhaseAbility->GetGamePhaseTag();

			/**
			 * If the active phase currently matches the incoming phase tag, we allow it.
			 * i.e. multiple gameplay abilities can all be associated with the same phase tag.
			 * For example, You can be in the Game.Playing phase and then start a sub-phase, like Game.Playing.SuddenDeath
			 * Game.Playing phase will still be active, and if someone were to push another one, like,
			 * Game.Playing.ActualSuddenDeath, it would end Game.Playing.SuddenDeath phase, but Game.Playing would
			 * continue. Similarly if we activated Game.GameOver, all the Game.Playing* phases would end.
			 */
			if (!IncomingPhaseTag.MatchesTag(ActivePhaseTag))
			{
				UE_LOG(LogGamePhase, Log, TEXT("\tEnding Phase '%s' (%s)"), *ActivePhaseTag.ToString(), *GetNameSafe(ActivePhaseAbility));

				FGameplayAbilitySpecHandle HandleToEnd = ActivePhase->Handle;
				GameState_ASC->CancelAbilitiesByFunc([HandleToEnd](const UCrimGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle) {
					return Handle == HandleToEnd;
				}, true);
			}
		}

		FCrimGamePhaseEntry& Entry = ActivePhaseMap.FindOrAdd(PhaseAbilityHandle);
		Entry.PhaseTag = IncomingPhaseTag;

		// Notify all observers of this phase that it has started.
		for (const FPhaseObserver& Observer : PhaseStartObservers)
		{
			if (Observer.IsMatch(IncomingPhaseTag))
			{
				Observer.PhaseCallback.ExecuteIfBound(IncomingPhaseTag);
			}
		}
	}
}

void UGamePhaseSubsystem::OnEndPhase(const UGamePhaseGameplayAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle)
{
	const FGameplayTag EndedPhaseTag = PhaseAbility->GetGamePhaseTag();
	UE_LOG(LogGamePhase, Log, TEXT("Ended Phase '%s' (%s)"), *EndedPhaseTag.ToString(), *GetNameSafe(PhaseAbility));

	const FCrimGamePhaseEntry& Entry = ActivePhaseMap.FindChecked(PhaseAbilityHandle);
	Entry.PhaseEndedCallback.ExecuteIfBound(PhaseAbility);

	ActivePhaseMap.Remove(PhaseAbilityHandle);

	// Notify all observers of this phase that it has ended.
	for (const FPhaseObserver& Observer : PhaseEndObservers)
	{
		if (Observer.IsMatch(EndedPhaseTag))
		{
			Observer.PhaseCallback.ExecuteIfBound(EndedPhaseTag);
		}
	}
}

bool UGamePhaseSubsystem::FPhaseObserver::IsMatch(const FGameplayTag& ComparePhaseTag) const
{
	switch(MatchType)
	{
	case EPhaseTagMatchType::ExactMatch:
		return ComparePhaseTag == PhaseTag;
	case EPhaseTagMatchType::PartialMatch:
		return ComparePhaseTag.MatchesTag(PhaseTag);
	}

	return false;
}
