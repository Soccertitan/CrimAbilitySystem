// Copyright Soccertitan 2025

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilityDefinition.generated.h"

class UGameplayAbility;

/**
 * Contains information about a GameplayAbility.
 */
UCLASS(Const)
class CRIMABILITYSYSTEM_API UGameplayAbilityDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UGameplayAbilityDefinition();
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Primary Asset")
	FPrimaryAssetType AssetType;

	// The UI facing name of the ability.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText AbilityName;

	// The UI facing icon of the ability.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;
	
	// The ability that is granted by this definition.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AssetBundles = "Ability"), AssetRegistrySearchable)
	TSoftClassPtr<UGameplayAbility> AbilityClass;
};
