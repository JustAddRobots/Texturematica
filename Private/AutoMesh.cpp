// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoMesh.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "HairStrandsInterface.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"

DEFINE_LOG_CATEGORY(LogAutoMesh);

class UMaterialFactoryNew;
// Sets default values
AAutoMesh::AAutoMesh()
{
 	// Set this actor to call Tick() every frame.
 	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAutoMesh::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAutoMesh::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TMap<FString, FString> AAutoMesh::GetAssetMap(UObject* Asset)
{
	checkf(Asset != nullptr, TEXT("nullptr: Asset"));
	
	TMap<FString, FString> AssetMap;
	const FString AssetObjectPath = FPackageName::GetNormalizedObjectPath(*Asset->GetPathName());
	const FString AssetObjectName = FPackageName::ObjectPathToObjectName(AssetObjectPath);
	const FString AssetPackagePath = FPackageName::GetLongPackagePath(*AssetObjectPath);
	const FString AssetPackageName = FPackageName::ObjectPathToPackageName(AssetObjectPath);
	AssetMap.Add(TEXT("ObjectPath"), AssetObjectPath);
	AssetMap.Add(TEXT("ObjectName"), AssetObjectName);
	AssetMap.Add(TEXT("PackagePath"), AssetPackagePath);
	AssetMap.Add(TEXT("PackageName"), AssetPackageName);
	return AssetMap;
}

UStaticMesh* AAutoMesh::GetStaticMesh(UObject* StaticMeshObject)
{
	checkf(StaticMeshObject != nullptr, TEXT("nullptr: StaticMeshObject"));

	UStaticMesh* StaticMesh = nullptr;
	if (StaticMeshObject->IsA(AStaticMeshActor::StaticClass()))
	{
		UE_LOG(LogAutoMesh, Warning, TEXT("Detected %s"), *StaticMeshObject->GetClass()->GetName())
		const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(StaticMeshObject);
		checkf(StaticMeshActor != nullptr, TEXT("nullptr: StaticMeshActor"));
		StaticMesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
		checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	}
	else if (StaticMeshObject->IsA(UStaticMeshComponent::StaticClass()))
	{
		const UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(StaticMeshObject);
		StaticMesh = StaticMeshComponent->GetStaticMesh();
	}
	else if (StaticMeshObject->IsA(UStaticMesh::StaticClass()))
	{
		StaticMesh = Cast<UStaticMesh>(StaticMeshObject);
	}
	else
	{
		UE_LOG(LogAutoMesh, Error, TEXT("Invalid Static Mesh Class: %s"), *StaticMeshObject->GetClass()->GetName());
	}

	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	return StaticMesh;
}

UMaterial* AAutoMesh::CreateMasterMaterial(UStaticMesh* StaticMesh)
{
	// Get master material path name from static mesh actor/object
	// e.g.: /Game/Meshes/Structure/SM_Structure_MeshName -> /Game/Materials/M_Structure
	
	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));

	TMap<FString, FString> StaticMeshMap = AAutoMesh::GetAssetMap(StaticMesh);
	const FString StaticMeshObjectPath = StaticMeshMap["ObjectPath"];
	const FString StaticMeshObjectName = StaticMeshMap["ObjectName"];
	const FString StaticMeshPackagePath = StaticMeshMap["PackagePath"];
	const FString StaticMeshPackageName = StaticMeshMap["PackageName"];
	
	TArray<FString> PackagePathArray;
	StaticMeshPackagePath.ParseIntoArray(
		PackagePathArray,
		TEXT("/"),
		true
	);
	const FString MaterialPackagePath = (
		TEXT("/") + PackagePathArray[0] + TEXT("/") + PackagePathArray[1]
	).Replace(
		TEXT("Meshes"),
		TEXT("Materials")
	);

	if (!FPackageName::IsValidPath(MaterialPackagePath))
	{
		UE_LOG(LogAutoMesh, Error, TEXT("Invalid Path: %s"), *MaterialPackagePath);
	}
	
	TArray<FString> ObjectNameArray;
	StaticMeshObjectName.ParseIntoArray(
		ObjectNameArray,
		TEXT("_"),
		true
	);

	const FString MaterialObjectName = ObjectNameArray[0].Replace(
		TEXT("SM"),
		TEXT("M")
	) + TEXT("_") + ObjectNameArray[1];

	const FString MaterialPackageName = FPackageName::GetNormalizedObjectPath(
		MaterialPackagePath + TEXT("/") + MaterialObjectName
	);
	
	if (!FPackageName::IsValidPath(MaterialPackageName))
	{
		UE_LOG(LogAutoMesh, Error, TEXT("Invalid PackageName: %s"), *MaterialPackageName);
	}
	
	UE_LOG(LogAutoMesh, Warning, TEXT("MaterialPackageName: %s"), *MaterialPackageName);

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString EngineContentDir = FPaths::EngineContentDir();
	EngineContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*EngineContentDir);
	
	// Load Material if already exists, otherwise create
	UMaterial* NewMaterial = nullptr;
	FString ExistingPackage; 
	if (FPackageName::DoesPackageExist(
		*MaterialPackageName,
		nullptr,
		&ExistingPackage) == true)
	{
		ExistingPackage = FileManager.ConvertToAbsolutePathForExternalAppForRead(*ExistingPackage);
		UE_LOG(LogAutoMesh, Warning, TEXT("Existing Package: %s"), *ExistingPackage);
		NewMaterial = LoadObject<UMaterial>(
			nullptr,
			*MaterialPackageName
		);	
	}
	else
	{
		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		NewMaterial = Cast<UMaterial>(
			AAutoMesh::CreateAsset(
				Factory,
				UMaterial::StaticClass(),
				MaterialObjectName,
				MaterialPackageName,
				MaterialPackagePath
			)
		);

		checkf(NewMaterial != nullptr, TEXT("nullptr: NewMaterial"));
		
		// Load engine textures used for MaterialExpressions
		UTexture* Texture127Grey = AAutoMesh::GetTexture(
			*EngineContentDir,
			TEXT("ArtTools/RenderToTexture/Textures/127grey.uasset")
		);
		
		UTexture* TextureNormalMap = AAutoMesh::GetTexture(
			*EngineContentDir,
			TEXT("EngineMaterials/BaseFlattenNormalMap.uasset")
		);

		// Create parameterized MaterialExpressions
		UMaterialExpressionTextureSampleParameter2D* DiffuseNode =
			NewObject<UMaterialExpressionTextureSampleParameter2D>(NewMaterial);
		UMaterialExpressionTextureSampleParameter2D* MaskNode =
			NewObject<UMaterialExpressionTextureSampleParameter2D>(NewMaterial);
		UMaterialExpressionTextureSampleParameter2D* NormalNode =
			NewObject<UMaterialExpressionTextureSampleParameter2D>(NewMaterial);
		DiffuseNode->ParameterName = FName(TEXT("Diffuse"));
		MaskNode->ParameterName = FName(TEXT("Mask"));
		NormalNode->ParameterName = FName(TEXT("Normal"));
		
		DiffuseNode->Texture = Texture127Grey;
		MaskNode->Texture = Texture127Grey;
		NormalNode->Texture = TextureNormalMap;

		DiffuseNode->MaterialExpressionEditorX = -300;
		DiffuseNode->MaterialExpressionEditorY = -100;
		MaskNode->MaterialExpressionEditorX = -300;
		MaskNode->MaterialExpressionEditorY = 200;
		NormalNode->MaterialExpressionEditorX = -300;
		NormalNode->MaterialExpressionEditorY = 500;
		
		DiffuseNode->SamplerType = SAMPLERTYPE_LinearColor;
		MaskNode->SamplerType = SAMPLERTYPE_LinearColor;
		NormalNode->SamplerType = SAMPLERTYPE_Normal;

		NewMaterial->Expressions.Add(DiffuseNode);
		NewMaterial->Expressions.Add(MaskNode);
		NewMaterial->Expressions.Add(NormalNode);

		NewMaterial->BaseColor.Expression = DiffuseNode;
		NewMaterial->Normal.Expression = NormalNode;

		NewMaterial->AmbientOcclusion.Expression = MaskNode;
		NewMaterial->AmbientOcclusion.Mask = 1;
		NewMaterial->AmbientOcclusion.MaskR = 1;
		
		NewMaterial->Roughness.Expression = MaskNode;
		NewMaterial->Roughness.Mask = 1;
		NewMaterial->Roughness.MaskG = 1;
		
		NewMaterial->Metallic.Expression = MaskNode;
		NewMaterial->Metallic.Mask = 1;
		NewMaterial->Metallic.MaskB = 1;

		NewMaterial->PostEditChange();
	}
	return NewMaterial;
}

UTexture* AAutoMesh::GetTexture(FString PrefixDir, const FString TextureFilename)
{
	checkf(*TextureFilename != nullptr, TEXT("nullptr: TextureFilename"));
	
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (!*PrefixDir)
	{
		PrefixDir = TEXT("");
	}
	FString TexturePath = FString::Printf(
		TEXT("%s%s"),
		*PrefixDir,
		*TextureFilename
	);
	
	TexturePath = FileManager.ConvertToAbsolutePathForExternalAppForRead(*TexturePath);
	UTexture* Texture = nullptr;
	if (FileManager.FileExists(*TexturePath))
	{
		const FString TexturePackageName = FPackageName::FilenameToLongPackageName(*TexturePath);
		UE_LOG(LogAutoMesh, Warning, TEXT("Loading Asset: %s"), *TexturePackageName);
		Texture = LoadObject<UTexture>(
			nullptr,
			*TexturePackageName,
			*TexturePath
		);
	}
	else
	{
		UE_LOG(LogAutoMesh, Error, TEXT("Not Exists: %s"), *TexturePath);
	}

	checkf(Texture != nullptr, TEXT("nullptr: Texture"));
	return Texture;
}

UMaterialInstanceConstant* AAutoMesh::CreateMaterialInstance(UMaterial* MasterMaterial, UStaticMesh* StaticMesh)
{
	checkf(MasterMaterial != nullptr, TEXT("nullptr: MasterMaterial"));
	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	
	TMap<FString, FString> StaticMeshMap = AAutoMesh::GetAssetMap(StaticMesh);
	const FString StaticMeshObjectPath = StaticMeshMap["ObjectPath"];
	const FString StaticMeshObjectName = StaticMeshMap["ObjectName"];
	const FString StaticMeshPackagePath = StaticMeshMap["PackagePath"];
	const FString StaticMeshPackageName = StaticMeshMap["PackageName"];
	const FString MaterialInstancePackagePath = *StaticMeshPackagePath.Replace(
		TEXT("Meshes"),
		TEXT("Materials")
	);
	const FString MaterialInstanceObjectName = *StaticMeshObjectName.Replace(
		TEXT("SM_"),
		TEXT("MI_")
	);
	const FString MaterialInstancePackageName = *StaticMeshPackageName.Replace(
		TEXT("Meshes"),
		TEXT("Materials")
	).Replace(
		TEXT("SM_"),
		TEXT("MI_")
	);

	UE_LOG(LogAutoMesh, Warning, TEXT("MaterialInstancePackageName: %s"), *MaterialInstancePackageName);

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = MasterMaterial;
	UMaterialInstanceConstant* NewMaterialInstance = Cast<UMaterialInstanceConstant>(
		AAutoMesh::CreateAsset(
			Factory,
			UMaterialInstanceConstant::StaticClass(),
			MaterialInstanceObjectName,
			MaterialInstancePackageName,
			MaterialInstancePackagePath
		)
	);
	NewMaterialInstance = AAutoMesh::AddTexturesToMIC(NewMaterialInstance, StaticMesh);
	checkf(NewMaterialInstance != nullptr, TEXT("nullptr: NewMaterialInstance"));
	return NewMaterialInstance;
}

UObject* AAutoMesh::CreateAsset(UFactory* Factory, UClass* StaticClass, const FString ObjectName,
	const FString PackageName, const FString PackagePath)
{
	checkf(Factory != nullptr, TEXT("nullptr: Factory"));
	checkf(StaticClass != nullptr, TEXT("nullptr: StaticClass"));
	checkf(*ObjectName != nullptr, TEXT("nullptr: ObjectName"));
	checkf(*PackageName != nullptr, TEXT("nullptr: PackageName"));
	checkf(*PackagePath != nullptr, TEXT("nullptr: PackagePath"));
	
	// Load modules for ContentBrowser updates
	const FAssetToolsModule& AssetToolsModule = FModuleManager::
		Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::
		LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::
		LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	UE_LOG(LogAutoMesh, Warning, TEXT("Creating Asset: %s"), *PackageName);
	UPackage* Package = CreatePackage(*PackageName);
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
		*ObjectName,
		*PackagePath,
		StaticClass,
		Factory
	);
	checkf(NewAsset != nullptr, TEXT("nullptr: NewAsset"));
	
	UE_LOG(LogAutoMesh, Warning, TEXT("Saving Package: %s"), *PackageName);
	UPackage::Save(
		Package,
		NewAsset,
		RF_Public | RF_Standalone,
		*FPackageName::LongPackageNameToFilename(
			*PackageName,
			*FPackageName::GetAssetPackageExtension()
		)
	);
	AssetRegistryModule.AssetCreated(NewAsset);
	TArray<UObject*> Objects;
	Objects.Add(NewAsset);
	ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
	return NewAsset;	
}

UMaterialInstanceConstant* AAutoMesh::AddTexturesToMIC(UMaterialInstanceConstant* MaterialInstance,
	UStaticMesh* StaticMesh)
{
	checkf(MaterialInstance != nullptr, TEXT("nullptr: MaterialInstance"));
	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	
	TMap<FString, FString> StaticMeshMap = AAutoMesh::GetAssetMap(StaticMesh);
	const FString StaticMeshObjectPath = StaticMeshMap["ObjectPath"];
	const FString StaticMeshObjectName = StaticMeshMap["ObjectName"];
	const FString StaticMeshPackagePath = StaticMeshMap["PackagePath"];
	const FString StaticMeshPackageName = StaticMeshMap["PackageName"];

	// Define standard UE texture parameters
	TArray<FName> DiffuseMaskNormal =
	{
		TEXT("Diffuse"),
		TEXT("Mask"),
		TEXT("Normal")
	};

	for (FName Param : DiffuseMaskNormal)
	{
		FString ParamStr;
		Param.ToString(ParamStr);
		
		FString TexturePackagePath = StaticMeshPackagePath.Replace(
			TEXT("Meshes"),
			TEXT("Textures")
		);

		FString TextureObjectName = StaticMeshObjectName.Replace(
			TEXT("SM_"),
			TEXT("T_")
		).Append(
			"_"
		).Append(
			*ParamStr.Left(1)  // Use first letter of param for texture suffix
		);

		FString TexturePackageName = StaticMeshPackageName.Replace(
			TEXT("SM_"),
			TEXT("T_")
		).Replace(
			TEXT("Meshes"),
			TEXT("Textures")
		).Append(
			"_"
		).Append(
			*ParamStr.Left(1)  // Use first letter of param for texture suffix
		);
		
		if (FPackageName::DoesPackageExist(*TexturePackageName))
		{
			UTexture* ParamTexture = LoadObject<UTexture>(
				nullptr,
				*TexturePackageName
			);
			
			if (ParamStr == "Mask")
			{
				ParamTexture->CompressionSettings = TC_Masks;
			}
			MaterialInstance->SetTextureParameterValueEditorOnly(Param, ParamTexture);
		}
		else
		{
			UE_LOG(LogAutoMesh, Error, TEXT("Not Exists: %s"), *TexturePackageName);
		}
	}
	return MaterialInstance;
}

UStaticMesh* AAutoMesh::AssignMaterial(UMaterialInstanceConstant* MaterialInstance, UStaticMesh* StaticMesh)
{
	checkf(MaterialInstance != nullptr, TEXT("nullptr: MaterialInstance"));
	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	
	StaticMesh->SetMaterial(
		0,
		MaterialInstance
	);
	return StaticMesh;
}