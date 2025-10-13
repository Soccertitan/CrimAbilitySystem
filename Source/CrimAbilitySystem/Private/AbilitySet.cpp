// Copyright Soccertitan


#include "AbilitySet.h"

#include "CrimAbilitySystemComponent.h"
#include "Ability/CrimGameplayAbility.h"
#include "Attribute/CrimAttributeSet.h"

void FAbilitySet_GameplayAbility::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s"), *GetNameSafe(Ability));
}

void FAbilitySet_GameplayEffect::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s"), *GetNameSafe(GameplayEffect));
}

void FAbilitySet_AttributeSet::PostSerialize(const FArchive& Ar)
{
	EditorDisplayName = FString::Printf(TEXT("%s"), *GetNameSafe(AttributeSet));
}

void FAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAbilitySet_GrantedHandles::AddAttributeSet(UCrimAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

void FAbilitySet_GrantedHandles::TakeFromAbilitySystem(UCrimAbilitySystemComponent* AbilitySystemComponent)
{
	check(AbilitySystemComponent);

	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UCrimAttributeSet* Set : GrantedAttributeSets)
	{
		AbilitySystemComponent->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UAbilitySet::UAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AssetType = TEXT("AbilitySet");
}

FPrimaryAssetId UAbilitySet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}

void UAbilitySet::GiveToAbilitySystem(UCrimAbilitySystemComponent* AbilitySystemComponent,
                                          FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AbilitySystemComponent);

	if (!AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		// Must be authoritative to give or take ability sets.
		return;
	}

	// Grant the attribute sets.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		UCrimAttributeSet* NewSet = NewObject<UCrimAttributeSet>(AbilitySystemComponent->GetOwner(), SetToGrant.AttributeSet);
		AbilitySystemComponent->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}

	// Grant the gameplay abilities.
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedGameplayAbilities[%d] on ability set [%s] is not valid."), AbilityIndex, *GetNameSafe(this));
			continue;
		}

		UCrimGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UCrimGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AppendTags(AbilityToGrant.DynamicTags);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant the gameplay effects.
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, AbilitySystemComponent->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}
