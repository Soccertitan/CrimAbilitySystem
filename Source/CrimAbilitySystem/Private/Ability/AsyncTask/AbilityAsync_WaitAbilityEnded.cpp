// Copyright Soccertitan 2025


#include "Ability/AsyncTask/AbilityAsync_WaitAbilityEnded.h"

#include "AbilitySystemComponent.h"


UAbilityAsync_WaitAbilityEnded* UAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedWithTags(
	AActor* TargetActor, FGameplayTagContainer AbilityTags, EGameplayContainerMatchType MatchType, bool bOnlyTriggerOnce)
{
	UAbilityAsync_WaitAbilityEnded* Obj = NewObject<UAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::ByTags;
	Obj->FilterTags = AbilityTags;
	Obj->FilterMatchType = MatchType;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

UAbilityAsync_WaitAbilityEnded* UAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedOfClass(
	AActor* TargetActor, TSubclassOf<UGameplayAbility> AbilityClass, bool bOnlyTriggerOnce)
{
	UAbilityAsync_WaitAbilityEnded* Obj = NewObject<UAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::ByClass;
	Obj->FilterAbilityClass = AbilityClass;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

UAbilityAsync_WaitAbilityEnded* UAbilityAsync_WaitAbilityEnded::WaitForAbilityEndedInstance(
	AActor* TargetActor, FGameplayAbilitySpecHandle SpecHandle, bool bOnlyTriggerOnce)
{
	UAbilityAsync_WaitAbilityEnded* Obj = NewObject<UAbilityAsync_WaitAbilityEnded>();
	Obj->SetAbilityActor(TargetActor);
	Obj->FilterMode = EWaitEndFilterMode::BySpecHandle;
	Obj->FilterSpecHandle = SpecHandle;
	Obj->bOnlyTriggerOnce = bOnlyTriggerOnce;
	return Obj;
}

void UAbilityAsync_WaitAbilityEnded::Activate()
{
	Super::Activate();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		MyHandle = ASC->OnAbilityEnded.AddUObject(this, &ThisClass::OnAbilityEnded);
	}
	else
	{
		EndAction();
	}
}

void UAbilityAsync_WaitAbilityEnded::EndAction()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->OnAbilityEnded.Remove(MyHandle);
	}
	Super::EndAction();
}

void UAbilityAsync_WaitAbilityEnded::OnAbilityEnded(const FAbilityEndedData& EndedData)
{
    if (!ShouldBroadcastDelegates())
    {
        EndAction();
        return;
    }

    UGameplayAbility* EndedAbility = EndedData.AbilityThatEnded;
    if (!EndedAbility)
    {
        return;
    }

    bool bMatch = false;

    switch (FilterMode)
    {
    case EWaitEndFilterMode::ByTags:
    {
        const FGameplayTagContainer& Tags = EndedAbility->GetAssetTags();
		switch (FilterMatchType)
		{
		case EGameplayContainerMatchType::Any:
			bMatch = Tags.HasAny(FilterTags);
			break;
		case EGameplayContainerMatchType::All:
			bMatch = Tags.HasAll(FilterTags);
			break;
		}
        break;
    }

    case EWaitEndFilterMode::ByClass:
    {
        bMatch = EndedAbility->IsA(FilterAbilityClass);
        break;
    }

    case EWaitEndFilterMode::BySpecHandle:
    {
        bMatch = (EndedData.AbilitySpecHandle == FilterSpecHandle);
        break;
    }
    }

    if (!bMatch)
    {
        return;
    }

    // Build and broadcast final data
    FCrimAbilityEndedData Out;
    Out.AbilityThatEnded = EndedAbility;
    Out.AbilitySpecHandle = EndedData.AbilitySpecHandle;
    Out.bWasCancelled = EndedData.bWasCancelled;

    Ended.Broadcast(Out);

    if (bOnlyTriggerOnce)
    {
        EndAction();
    }
}