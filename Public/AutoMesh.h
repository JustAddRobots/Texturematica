// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceConstant.h"
#include "AutoMesh.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAutoMesh, Log, All);

/**
 * This class helps automate the pipeline detailed in the Epic Games course
 * "Build a Detective's Office Game Environment".
 *
 * https://dev.epicgames.com/community/learning/courses/WK/unreal-engine-build-a-detective-s-office-game-environment
 *
 * Meshes and their associated textures are used to generate material instances which
 * are subsequently assigned back to the mesh. This class is intended to be
 * used in Blueprint and expects the file layout from the course. i.e:
 * 
 * /Game/Meshes/[Prop|Structure]/SM_[Prop|Structure]_MeshName
 * /Game/Textures/[Prop|Structure]/T_[Prop|Structure]_MeshName_[D|M|N]
 * /Game/Materials/[Prop|Structure]/
 */
UCLASS()
class TEXTUREMATICA_API AAutoMesh : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAutoMesh();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * Get a map of asset's package and object information with the following keys:
	 *	"Object Name", "Object Path", "Package Name", "Package Path"
	 *	@param Asset - Asset for which to retrieve information.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static TMap<FString, FString> GetAssetMap(UObject* Asset);
	
	/**
	 * Get static mesh from AStaticMeshActor, UStaticMeshComponent, or UStaticMesh object.
	 * @param StaticMeshObject - Static mesh.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UStaticMesh* GetStaticMesh(UObject* StaticMeshObject);
	
	/**
	 * Create master material with UE standard "Diffuse", "Mask", & "Normal" texture parameters.
	 * @param StaticMesh - Static mesh for which to create material.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UMaterial* CreateMasterMaterial(UStaticMesh* StaticMesh);

	/**
	 * Get texture using filesystem pathname.
	 * @param PrefixDir - Optional prefix path for TextureFilename.
	 * @param TextureFilename - Filename of texture asset.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UTexture* GetTexture(FString PrefixDir, FString TextureFilename);

	/**
	 * Create material instance from parent material and static mesh object path.
	 * @param MasterMaterial - Parent material, assumes "Diffuse", "Mask", "Normal" parameters
	 * @param StaticMesh - Mesh object from which to derive path for material instance and textures.
	*/
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UMaterialInstanceConstant* CreateMaterialInstance(UMaterial* MasterMaterial, UStaticMesh* StaticMesh);

	/**
	 * Create asset from factory and object data. Generalised to create different kinds of objects.
	 * @param Factory - Factory used to create new instance.
	 * @param StaticClass - Class from which to create asset.
	 * @param ObjectName - Object name of asset.
	 * @param PackageName - Package name of asset.
	 * @param PackagePath - Package path of asset.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UObject* CreateAsset(UFactory* Factory, UClass* StaticClass, FString ObjectName, FString PackageName,
		FString PackagePath);

	/**
	 * Add textures to material instance.
	 * @param MaterialInstance - Instance with "Diffuse", "Mask", "Normal" texture parameters.
	 * @param StaticMesh - Mesh object from which to derive paths for textures.
	 */	
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UMaterialInstanceConstant* AddTexturesToMIC(UMaterialInstanceConstant* MaterialInstance,
		UStaticMesh* StaticMesh);
	
	/**
	 * Assign material instance to static mesh.
	 * @param MaterialInstance - Material instance to assign.
	 * @param StaticMesh - Object path of static mesh.
	 */
	UFUNCTION(BlueprintCallable, Category="AutoMesh")
	static UStaticMesh* AssignMaterial(UMaterialInstanceConstant* MaterialInstance, UStaticMesh* StaticMesh);
};
