// Copyright Soccertitan 2025


#include "CrimAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CrimAbilitySystemInterface.h"
#include "CrimAbilityLogChannels.h"
#include "CrimGlobalAbilitySystem.h"
#include "AbilityTagRelationshipMapping.h"


UCrimAbilitySystemComponent::UCrimAbilitySystemComponent()
{
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UCrimAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UCrimGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrimGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterAbilitySystemComponent(this);
	}
	
	Super::EndPlay(EndPlayReason);
}

void UCrimAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (bHasNewPawnAvatar)
	{
		ABILITYLIST_SCOPE_LOCK();
		// Notify all abilities that a new pawn avatar has been set
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec.Ability);
			if (!CrimAbilityCDO)
			{
				continue;
			}

			if (CrimAbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor ||
				CrimAbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution)
			{
				TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
				for (UGameplayAbility* AbilityInstance : Instances)
				{
					UCrimGameplayAbility* CrimAbilityInstance = Cast<UCrimGameplayAbility>(AbilityInstance);
					if (CrimAbilityInstance)
					{
						// Ability instances may be missing for replays
						CrimAbilityInstance->OnPawnAvatarSet();
					}
				}
			}
			else
			{
				CrimAbilityCDO->OnPawnAvatarSet();
			}
		}
	}
	
	if (UCrimGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrimGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->RegisterAbilitySystemComponent(this);
	}

	TArray<UActorComponent*> ActorComponents = InOwnerActor->GetComponentsByInterface(UCrimAbilitySystemInterface::StaticClass());
	for (UActorComponent*& Component : ActorComponents)
	{
		ICrimAbilitySystemInterface::Execute_InitializeWithAbilitySystem(Component, this);
	}

	if (InOwnerActor != InAvatarActor)
	{
		ActorComponents = InAvatarActor->GetComponentsByInterface(UCrimAbilitySystemInterface::StaticClass());
		for (UActorComponent*& Component : ActorComponents)
		{
			ICrimAbilitySystemInterface::Execute_InitializeWithAbilitySystem(Component, this);
		}
	}

	TryActivateAbilitiesOnSpawn();
}

void UCrimAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec.Ability);
		if (!CrimAbilityCDO)
		{
			UE_LOG(LogCrimAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Non-CrimGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		if (CrimAbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerActor ||
			CrimAbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution)
		{
			// Cancel all the spawned instances, not the CDO.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UCrimGameplayAbility* CrimAbilityInstance = CastChecked<UCrimGameplayAbility>(AbilityInstance);

				if (ShouldCancelFunc(CrimAbilityInstance, AbilitySpec.Handle))
				{
					if (CrimAbilityInstance->CanBeCanceled())
					{
						CrimAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), CrimAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						UE_LOG(LogCrimAbilitySystem, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *CrimAbilityInstance->GetName());
					}
				}
			}
		}
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(CrimAbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(CrimAbilityCDO->CanBeCanceled());
				CrimAbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
	}
}

bool UCrimAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
			bBlocked = false;
		break;

	case EAbilityActivationGroup::Exclusive_Replaceable:
	case EAbilityActivationGroup::Exclusive_Blocking:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)EAbilityActivationGroup::Exclusive_Blocking] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UCrimAbilitySystemComponent::AddAbilityToActivationGroup(EAbilityActivationGroup Group, UCrimGameplayAbility* CrimAbility)
{
	check(CrimAbility);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
			break;

	case EAbilityActivationGroup::Exclusive_Replaceable:
	case EAbilityActivationGroup::Exclusive_Blocking:
		CancelActivationGroupAbilities(EAbilityActivationGroup::Exclusive_Replaceable, CrimAbility, bReplicateCancelAbility);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EAbilityActivationGroup::Exclusive_Replaceable] + ActivationGroupCounts[(uint8)EAbilityActivationGroup::Exclusive_Blocking];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogCrimAbilitySystem, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UCrimAbilitySystemComponent::RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, UCrimGameplayAbility* CrimAbility)
{
	check(CrimAbility);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UCrimAbilitySystemComponent::CancelActivationGroupAbilities(EAbilityActivationGroup Group, UCrimGameplayAbility* IgnoreCrimAbility, bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreCrimAbility](const UCrimGameplayAbility* CrimAbility, FGameplayAbilitySpecHandle Handle)
	{
		return ((CrimAbility->GetActivationGroup() == Group) && (CrimAbility != IgnoreCrimAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UCrimAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

void UCrimAbilitySystemComponent::SetTagRelationshipMapping(UAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UCrimAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

FGameplayAbilitySpec* UCrimAbilitySystemComponent::GetAbilitySpecWithAbilityTag(const FGameplayTag& AbilityTag)
{
	if (AbilityTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.Ability.Get()->GetAssetTags().HasTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UCrimAbilitySystemComponent::GetAllAbilitySpecsWithAbilityTag(const FGameplayTag& AbilityTag, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs)
{
	OutAbilitySpecs.Empty();
	if (AbilityTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.Ability.Get()->GetAssetTags().HasTag(AbilityTag))
			{
				OutAbilitySpecs.Add(&AbilitySpec);
			}
		}
	}
}

FGameplayAbilitySpec* UCrimAbilitySystemComponent::GetAbilitySpecWithDynamicTag(const FGameplayTag& DynamicTag)
{
	if (DynamicTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(DynamicTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UCrimAbilitySystemComponent::GetAllAbilitySpecsWithDynamicTag(const FGameplayTag& DynamicTag, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs)
{
	OutAbilitySpecs.Empty();
	if (DynamicTag.IsValid())
	{
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.GetDynamicSpecSourceTags().HasTag(DynamicTag))
			{
				OutAbilitySpecs.Add(&AbilitySpec);
			}
		}
	}
}

FGameplayAbilitySpec* UCrimAbilitySystemComponent::GetAbilitySpecByClass(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (AbilityClass)
	{
		const UGameplayAbility* const InAbilityCDO = AbilityClass.GetDefaultObject();
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.Ability == InAbilityCDO)
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UCrimAbilitySystemComponent::GetAllAbilitySpecsByClass(TSubclassOf<UGameplayAbility> AbilityClass, TArray<FGameplayAbilitySpec*>& OutAbilitySpecs)
{
	OutAbilitySpecs.Empty();
	if (AbilityClass)
	{
		const UGameplayAbility* const InAbilityCDO = AbilityClass.GetDefaultObject();
		FScopedAbilityListLock ActiveScopeLoc(*this);
		for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
		{
			if (AbilitySpec.Ability == InAbilityCDO)
			{
				OutAbilitySpecs.Add(&AbilitySpec);
			}
		}
	}
}

void UCrimAbilitySystemComponent::AddDynamicTagToAbilitySpec(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& DynamicTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!AbilitySpec)
	{
		return;
	}

	if (AbilitySpec->GetDynamicSpecSourceTags().HasTag(DynamicTag))
	{
		// Dynamic Spec Source tags already has the DynamicTag.
		return;
	}

	TArray<FGameplayAbilitySpec*> OutSpecs;
	GetAllAbilitySpecsWithDynamicTag(DynamicTag, OutSpecs);
	for (FGameplayAbilitySpec*& Spec : OutSpecs)
	{
		Spec->GetDynamicSpecSourceTags().RemoveTag(DynamicTag);
		MarkAbilitySpecDirty(*Spec);
	}
	
	AbilitySpec->GetDynamicSpecSourceTags().AddTag(DynamicTag);
	MarkAbilitySpecDirty(*AbilitySpec);
}

void UCrimAbilitySystemComponent::RemoveDynamicTagFromAbilitySpec(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& DynamicTag)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!AbilitySpec)
	{
		return;
	}

	if (!AbilitySpec->GetDynamicSpecSourceTags().HasTag(DynamicTag))
	{
		// Dynamic Spec Source tags already does not have the tag.
		return;
	}

	AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(DynamicTag);
	MarkAbilitySpecDirty(*AbilitySpec);
}

void UCrimAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UCrimGameplayAbility* CrimAbilityCDO = Cast<UCrimGameplayAbility>(AbilitySpec.Ability))
		{
			CrimAbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UCrimAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed,
			Spec.Handle,
			Spec.GetPrimaryInstance()->GetCurrentActivationInfo().GetActivationPredictionKey()
		);
	}
}

void UCrimAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased,
			Spec.Handle,
			Spec.GetPrimaryInstance()->GetCurrentActivationInfo().GetActivationPredictionKey()
		);
	}
}

void UCrimAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);
	
	if (UCrimGameplayAbility* CrimAbility = Cast<UCrimGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(CrimAbility->GetActivationGroup(), CrimAbility);
	}
}

void UCrimAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	if (APawn* Avatar = Cast<APawn>(GetAvatarActor()))
	{
		if (!Avatar->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UCrimAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UCrimGameplayAbility* CrimAbility = Cast<UCrimGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(CrimAbility->GetActivationGroup(), CrimAbility);
	}
}

void UCrimAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags,
	bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);
}

void UCrimAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);
}

void UCrimAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	OnAbilityGivenDelegate.Broadcast(this, AbilitySpec);
}

void UCrimAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
	OnAbilityRemovedDelegate.Broadcast(this, AbilitySpec);
}

void UCrimAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UCrimAbilitySystemComponent::HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	if (const UCrimGameplayAbility* CrimAbility = Cast<const UCrimGameplayAbility>(Ability))
	{
		CrimAbility->OnAbilityFailedToActivate(FailureReason);
	}
}
