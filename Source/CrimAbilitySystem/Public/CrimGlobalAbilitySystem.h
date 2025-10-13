// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "CrimGlobalAbilitySystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UCrimAbilitySystemComponent;
class UObject;

USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UCrimAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToAbilitySystemComponent(TSubclassOf<UGameplayAbility> Ability, UCrimAbilitySystemComponent* AbilitySystemComponent);
	void RemoveFromAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent);
	void RemoveFromAll();
};

USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<UCrimAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToAbilitySystemComponent(TSubclassOf<UGameplayEffect> Effect, UCrimAbilitySystemComponent* AbilitySystemComponent);
	void RemoveFromAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent);
	void RemoveFromAll();
};

/**
 * 
 */
UCLASS()
class CRIMABILITYSYSTEM_API UCrimGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	
	UCrimGlobalAbilitySystem();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Crim Ability System|Global")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Crim Ability System|Global")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Global")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Global")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** Register an ASC with global system and apply any active global effects/abilities. */
	void RegisterAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent);

	/** Removes an ASC from the global system, along with any active global effects/abilities. */
	void UnregisterAbilitySystemComponent(UCrimAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UCrimAbilitySystemComponent>> RegisteredAbilitySystemComponents;
};
