// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilitySet.generated.h"


class UCrimAttributeSet;
class UCrimAbilitySystemComponent;
class UGameplayEffect;
class UCrimGameplayAbility;

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCrimGameplayAbility> Ability = nullptr;

	// Level of the ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Dynamic GameplayTags to add to the AbilitySpec.
	UPROPERTY(EditDefaultsOnly)
	FGameplayTagContainer DynamicTags;

#if WITH_EDITORONLY_DATA
	// Without an attribute such as VisibleAnywhere the editor TitleProperty code won't be able to find this property.
	UPROPERTY(VisibleAnywhere, meta=(EditCondition="false", EditConditionHides))
	FString EditorDisplayName;
#endif

	void PostSerialize(const FArchive& Ar);
};
template<>
struct TStructOpsTypeTraits<FAbilitySet_GameplayAbility> : public TStructOpsTypeTraitsBase2<FAbilitySet_GameplayAbility>
{
	enum
	{
		WithPostSerialize = true,
   };
};

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	// Gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Level of gameplay effect to grant.
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.f;

#if WITH_EDITORONLY_DATA
	// Without an attribute such as VisibleAnywhere the editor TitleProperty code won't be able to find this property.
	UPROPERTY(VisibleAnywhere, meta=(EditCondition="false", EditConditionHides))
	FString EditorDisplayName;
#endif

	void PostSerialize(const FArchive& Ar);
};
template<>
struct TStructOpsTypeTraits<FAbilitySet_GameplayEffect> : public TStructOpsTypeTraitsBase2<FAbilitySet_GameplayEffect>
{
	enum
	{
		WithPostSerialize = true,
   };
};

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilitySet_AttributeSet
{
	GENERATED_BODY()

	// Attribute set to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCrimAttributeSet> AttributeSet = nullptr;

#if WITH_EDITORONLY_DATA
	// Without an attribute such as VisibleAnywhere the editor TitleProperty code won't be able to find this property.
	UPROPERTY(VisibleAnywhere, meta=(EditCondition="false", EditConditionHides))
	FString EditorDisplayName;
#endif

	void PostSerialize(const FArchive& Ar);
};
template<>
struct TStructOpsTypeTraits<FAbilitySet_AttributeSet> : public TStructOpsTypeTraitsBase2<FAbilitySet_AttributeSet>
{
	enum
	{
		WithPostSerialize = true,
   };
};

USTRUCT(BlueprintType)
struct CRIMABILITYSYSTEM_API FAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UCrimAttributeSet* Set);

	void TakeFromAbilitySystem(UCrimAbilitySystemComponent* AbilitySystemComponent);

protected:

	// Handles to the granted abilities.
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	// Handles to the granted gameplay effects.
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	// Pointers to the granted attribute sets
	UPROPERTY()
	TArray<TObjectPtr<UCrimAttributeSet>> GrantedAttributeSets;
};

/**
 * Non-mutable data asset used to grant gameplay abilities, effects, and attribute sets.
 */
UCLASS(BlueprintType, Const)
class CRIMABILITYSYSTEM_API UAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Primary Asset")
	FPrimaryAssetType AssetType;

	/**
	 * Grants the ability set to the specified ability system component.
	 * The returned handles can be used later to take away anything that was granted.
	 */
	void GiveToAbilitySystem(UCrimAbilitySystemComponent* AbilitySystemComponent, FAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;
	
protected:

	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=EditorDisplayName))
	TArray<FAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	// Gameplay effects to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=EditorDisplayName))
	TArray<FAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=EditorDisplayName))
	TArray<FAbilitySet_AttributeSet> GrantedAttributes;
};
