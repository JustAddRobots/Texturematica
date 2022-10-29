// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoMesh.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "EditorTutorial.h"
// #include "HairStrandsInterface.h"
#include "IContentBrowserSingleton.h"
#include "PackageTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Logging/TokenizedMessage.h"
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

FString AAutoMesh::GetIniValue(const FString IniFile, const FString Section, const FString Key)
{
	FString Value;
	if (GConfig->GetString(
		*Section,
		*Key,
		Value,
		IniFile
	))
	{
		UE_LOG(LogAutoMesh, Warning, TEXT("Detected INI, %s: %s"), *Key, *Value);
	}
	return Value;
}

TMap<FString, FString> AAutoMesh::GetIniValues()
{
	TMap<FString, FString> IniValues;
	if (GConfig)
	{
		const FString IniFile = FPaths::ProjectConfigDir().Append("DefaultTexturematica.ini");
		const FString Section = TEXT("/Script/Texturematica.AutoMesh");
		const FString MaterialsDir = AAutoMesh::GetIniValue(IniFile, Section, TEXT("MaterialsDir"));
		const FString MeshesDir = AAutoMesh::GetIniValue(IniFile, Section, TEXT("MeshesDir"));
		const FString TexturesDir = AAutoMesh::GetIniValue(IniFile, Section, TEXT("TexturesDir"));
		IniValues.Add(TEXT("MaterialsDir"), MaterialsDir);
		IniValues.Add(TEXT("MeshesDir"), MeshesDir);
		IniValues.Add(TEXT("TexturesDir"), TexturesDir);
	}
	return IniValues;
}

TMap<FString, FString> AAutoMesh::GetAssetMap(UObject* Asset)
{
	TMap<FString, FString> AssetMap;
	if (!IsValid(Asset)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Asset")) }
	else
	{
		const FString AssetObjectPath = FPackageName::GetNormalizedObjectPath(*Asset->GetPathName());
		const FString AssetObjectName = FPackageName::ObjectPathToObjectName(AssetObjectPath);
		const FString AssetPackagePath = FPackageName::GetLongPackagePath(*AssetObjectPath);
		const FString AssetPackageName = FPackageName::ObjectPathToPackageName(AssetObjectPath);
		AssetMap.Add(TEXT("ObjectPath"), AssetObjectPath);
		AssetMap.Add(TEXT("ObjectName"), AssetObjectName);
		AssetMap.Add(TEXT("PackagePath"), AssetPackagePath);
		AssetMap.Add(TEXT("PackageName"), AssetPackageName);
	}
	return AssetMap;
}

UStaticMesh* AAutoMesh::GetStaticMesh(UObject* StaticMeshObject)
{
	UStaticMesh* StaticMesh = nullptr;
	if (!IsValid(StaticMeshObject)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMeshObject")) }
	else
	{
		if (StaticMeshObject->IsA(AStaticMeshActor::StaticClass()))
		{
			const AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(StaticMeshObject);
			StaticMesh = StaticMeshActor->GetStaticMeshComponent()->GetStaticMesh();
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
	}
	return StaticMesh;
}

UMaterial* AAutoMesh::CreateMasterMaterial(UStaticMesh* StaticMesh)
{
	UMaterial* NewMaterial = nullptr;
	if (!IsValid(StaticMesh))
	{
		UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMesh"))
	}
	else
	{
		TMap<FString, FString> StaticMeshMap = AAutoMesh::GetAssetMap(StaticMesh);
		const FString StaticMeshObjectPath = StaticMeshMap["ObjectPath"];
		const FString StaticMeshObjectName = StaticMeshMap["ObjectName"];
		const FString StaticMeshPackagePath = StaticMeshMap["PackagePath"];
		const FString StaticMeshPackageName = StaticMeshMap["PackageName"];

		// Get plugin defaults from CDO
		const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());
		UE_LOG(LogAutoMesh, Warning, TEXT("StaticMeshPackageName: %s"), *StaticMeshPackageName)
		UE_LOG(LogAutoMesh, Warning, TEXT("StaticMeshObjectName: %s"), *StaticMeshObjectName)

		if (!*StaticMeshObjectPath) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMeshObjectPath")) }
		else if (!*StaticMeshObjectName) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMeshObjectName")) }
		else if (!*StaticMeshPackagePath) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMeshPackagePath")) }
		else if (!*StaticMeshPackageName) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMeshPackageName")) }
		else
		{
			// Use StaticMesh path info as template for Material path
			// e.g.: /Game/Meshes/Structure/SM_Structure_MeshName -> /Game/Materials/M_Structure
			TArray<FString> StaticMeshPackagePathArray;
			StaticMeshPackagePath.ParseIntoArray(
				StaticMeshPackagePathArray,
				TEXT("/"),
				true
			);
			const FString MaterialPackagePath = (
				TEXT("/") + StaticMeshPackagePathArray[0] + TEXT("/") + StaticMeshPackagePathArray[1]
			).Replace(
				*AutoMeshDefault->MeshesDir,
				*AutoMeshDefault->MaterialsDir
			);

			TArray<FString> StaticMeshObjectNameArray;
			StaticMeshObjectName.ParseIntoArray(
				StaticMeshObjectNameArray,
				TEXT("_"),
				true
			);
			const FString MaterialObjectName = StaticMeshObjectNameArray[0].Replace(
				TEXT("SM"),
				TEXT("M")
			) + TEXT("_") + StaticMeshObjectNameArray[1];
			const FString MaterialPackageName = FPackageName::GetNormalizedObjectPath(
				MaterialPackagePath + TEXT("/") + MaterialObjectName
			);

			if (!FPackageName::IsValidPath(MaterialPackagePath))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("Invalid MaterialPackagePath: %s"), *MaterialPackagePath);
			}
			else if (!FPackageName::IsValidPath(MaterialPackageName))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("Invalid MaterialPackageName: %s"), *MaterialPackageName);
			}
			else
			{
				// UE_LOG(LogAutoMesh, Warning, TEXT("MaterialPackageName: %s"), *MaterialPackageName);

				IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
				FString EngineContentDir = FPaths::EngineContentDir();
				EngineContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*EngineContentDir);

				// Load Material if already exists, otherwise create
				FString ExistingPackage;
				if (FPackageName::DoesPackageExist(
					*MaterialPackageName,
					nullptr,
					&ExistingPackage
				) == true)
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

					// Load engine textures used for MaterialExpressions
					UTexture* Texture127Grey = AAutoMesh::GetTexture(
						*EngineContentDir,
						TEXT("ArtTools/RenderToTexture/Textures/127grey.uasset")
					);

					UTexture* TextureNormalMap = AAutoMesh::GetTexture(
						*EngineContentDir,
						TEXT("EngineMaterials/BaseFlattenNormalMap.uasset")
					);

					if (!IsValid(NewMaterial)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: NewMaterial")) }
					else if (!IsValid(Texture127Grey)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Texture127Grey")) }
					else if (!IsValid(TextureNormalMap))
					{
						UE_LOG(LogAutoMesh, Error, TEXT("nullptr: TextureNormalMap"))
					}
					else
					{
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
				}
			}
		}
	}
	return NewMaterial;
}

UTexture* AAutoMesh::GetTexture(FString PrefixDir, const FString TextureFilename)
{
	UTexture* Texture = nullptr;
	if (!*TextureFilename) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: TextureFilename")) }
	else
	{
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
	}
	return Texture;
}

UMaterialInstanceConstant* AAutoMesh::CreateMaterialInstance(UMaterial* MasterMaterial, UStaticMesh* StaticMesh)
{
	const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
	UMaterialInstanceConstant* NewMaterialInstance = nullptr;
	if (!IsValid(MasterMaterial)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: MasterMaterial")) }
	else if (!IsValid(StaticMesh)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticMesh")) }
	else
	{
		TMap<FString, FString> StaticMeshMap = AAutoMesh::GetAssetMap(StaticMesh);
		const FString StaticMeshObjectPath = StaticMeshMap["ObjectPath"];
		const FString StaticMeshObjectName = StaticMeshMap["ObjectName"];
		const FString StaticMeshPackagePath = StaticMeshMap["PackagePath"];
		const FString StaticMeshPackageName = StaticMeshMap["PackageName"];

		const FString MaterialInstancePackagePath = *StaticMeshPackagePath.Replace(
			*AutoMeshDefault->MeshesDir,
			*AutoMeshDefault->MaterialsDir
		);
		const FString MaterialInstanceObjectName = *StaticMeshObjectName.Replace(
			TEXT("SM_"),
			TEXT("MI_")
		);
		const FString MaterialInstancePackageName = *StaticMeshPackageName.Replace(
			*AutoMeshDefault->MeshesDir,
			*AutoMeshDefault->MaterialsDir
		).Replace(
			TEXT("SM_"),
			TEXT("MI_")
		);

		UE_LOG(LogAutoMesh, Warning, TEXT("MaterialInstancePackageName: %s"), *MaterialInstancePackageName);

		UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
		Factory->InitialParent = MasterMaterial;
		NewMaterialInstance = Cast<UMaterialInstanceConstant>(
			AAutoMesh::CreateAsset(
				Factory,
				UMaterialInstanceConstant::StaticClass(),
				MaterialInstanceObjectName,
				MaterialInstancePackageName,
				MaterialInstancePackagePath
			)
		);
		if (!IsValid(NewMaterialInstance)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: NewMaterialInstance")) }
		else
		{
			NewMaterialInstance = AAutoMesh::AddTexturesToMIC(NewMaterialInstance, StaticMesh);
		}
	}
	return NewMaterialInstance;
}

UObject* AAutoMesh::CreateAsset(UFactory* Factory, UClass* StaticClass, const FString ObjectName,
	const FString PackageName, const FString PackagePath)
{
	UObject* Asset = nullptr;
	if (!IsValid(Factory)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Factory")) }
	else if (!IsValid(StaticClass)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: StaticClass")) }
	else if (!*ObjectName) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: ObjectName")) }
	else if (!*PackageName) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: PackageName")) }
	else if (!*PackagePath) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: PackagePath")) }
	else
	{
		// Load modules for ContentBrowser updates
		const FAssetToolsModule& AssetToolsModule = FModuleManager::
			Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		const FContentBrowserModule& ContentBrowserModule = FModuleManager::
			LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::
			LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		UE_LOG(LogAutoMesh, Warning, TEXT("PackagePath: %s"), *PackagePath);
		UE_LOG(LogAutoMesh, Warning, TEXT("ObjectName: %s"), *ObjectName);
		UE_LOG(LogAutoMesh, Warning, TEXT("PackageName: %s"), *PackageName);
		UE_LOG(LogAutoMesh, Warning, TEXT("StaticClass: %s"), *StaticClass->GetName());
		UE_LOG(LogAutoMesh, Warning, TEXT("Factory: %s"), *Factory->GetName());
		
		FString ExistingPackage;
		if (FPackageName::DoesPackageExist(
			*PackageName,
			nullptr,
			&ExistingPackage)
		)
		{
			IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
			ExistingPackage = FileManager.ConvertToAbsolutePathForExternalAppForRead(*ExistingPackage);
			UE_LOG(LogAutoMesh, Warning, TEXT("Existing Package: %s"), *ExistingPackage);
			UPackage* Package = UPackageTools::LoadPackage(*PackageName);
			Package->FullyLoad();
			Asset =	StaticLoadObject(
				StaticClass,
				Package,
				*ObjectName
			);
		}
		else
		{
			UE_LOG(LogAutoMesh, Warning, TEXT("Creating Asset: %s"), *PackageName);
			UPackage* Package = CreatePackage(*PackageName);
			Asset = AssetToolsModule.Get().CreateAsset(
				*ObjectName,
				*PackagePath,
				StaticClass,
				Factory
			);

			if (!IsValid(Asset)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Asset")) }
			else if (!IsValid(Package)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Package")) }
			else
			{
				UE_LOG(LogAutoMesh, Warning, TEXT("Saving Package: %s"), *PackageName);
				UPackage::Save(
					Package,
					Asset,
					RF_Public | RF_Standalone,
					*FPackageName::LongPackageNameToFilename(
						*PackageName,
						*FPackageName::GetAssetPackageExtension()
					)
				);
				AssetRegistryModule.AssetCreated(Asset);
				TArray<UObject*> Objects;
				Objects.Add(Asset);
				ContentBrowserModule.Get().SyncBrowserToAssets(Objects);
			}
		}
	}
	return Asset;	
}

UMaterialInstanceConstant* AAutoMesh::AddTexturesToMIC(UMaterialInstanceConstant* MaterialInstance,
	UStaticMesh* StaticMesh)
{
	checkf(MaterialInstance != nullptr, TEXT("nullptr: MaterialInstance"));
	checkf(StaticMesh != nullptr, TEXT("nullptr: StaticMesh"));
	
	// Get plugin defaults from CDO
	const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());
	
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
			*AutoMeshDefault->MeshesDir,
			*AutoMeshDefault->TexturesDir
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
			*AutoMeshDefault->MeshesDir,
			*AutoMeshDefault->TexturesDir
		).Append(
			"_"
		).Append(
			*ParamStr.Left(1)  // Use first letter of param for texture suffix
		);

		UE_LOG(LogAutoMesh, Warning, TEXT("TexturePackageName: %s"), *TexturePackageName);
		UPackage* TexturePackage = UPackageTools::LoadPackage(*TexturePackageName);
		UObject* TextureObject = AAutoMesh::GetRenamedObject(TexturePackage);
		if (FPackageName::DoesPackageExist(*TexturePackageName))
		{
			UTexture* ParamTexture = LoadObject<UTexture>(
				TexturePackage,
				*TextureObject->GetName()
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

UObject* AAutoMesh::GetRenamedObject(UPackage* Package)
{
	UObject* Object = nullptr;
	if (!IsValid(Package)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Package")); }
	else
	{
		Package->FullyLoad();
		TArray<UObject*> Objects;
		TArray<UPackage*> Packages;
		Packages.Add(Package);
		UPackageTools::GetObjectsInPackages(&Packages, Objects);
		Object = Objects[0];
		Object->Rename(*FPackageName::GetShortName(*Package->GetName())); // Rename to match package
		UE_LOG(LogAutoMesh, Warning, TEXT("PackageName: %s"), *Package->GetName());
		UE_LOG(LogAutoMesh, Warning, TEXT("ObjectName: %s"), *Object->GetName());
	}
	return Object;
}

bool AAutoMesh::SavePackage(UPackage* Package, UObject* Object)
{
	bool bIsSaveSuccessful = false;
	if (!IsValid(Package)) { UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Package")); }
	else if (!IsValid(Object)) {UE_LOG(LogAutoMesh, Error, TEXT("nullptr: Object")); }
	else
	{
		bIsSaveSuccessful = UPackage::SavePackage(
			Package,
			Object,
			RF_Public | RF_Standalone,
			*FPackageName::LongPackageNameToFilename(
				*Package->GetName(),
				*FPackageName::GetAssetPackageExtension()
			)
		);
	}
	if (bIsSaveSuccessful) { UE_LOG(LogAutoMesh, Warning, TEXT("Saved Package: %s"), *Package->GetName()) }
	else { UE_LOG(LogAutoMesh, Error, TEXT("Package Save Failure")) }
	return bIsSaveSuccessful;
}

TMap<FString, FString> AAutoMesh::GetMockAssetMap(FString ContentSubDir)
{
	TMap<FString, FString> MockAssetMap;
	FString Timestamp = FDateTime::Now().ToString().Replace(TEXT("."), TEXT(""));
	if (*ContentSubDir == TEXT(""))
	{
		ContentSubDir = Timestamp;
	}
	const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

	// Dirs
	FString ContentDir = FString::Printf(
		TEXT("%s%s/%s/%s/"),
		*FPaths::ProjectPluginsDir(),
		TEXT("Texturematica"),
		TEXT("Content"),
		*ContentSubDir
	);
	FString PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*ContentDir);
	const FString MeshesDir = FString::Printf(
		TEXT("%s%s/"),
		*PlatformContentDir,
		*AutoMeshDefault->MeshesDir
	);
	const FString MeshesSubDir = FString::Printf(
		TEXT("%s%s/"),
		*MeshesDir,
		TEXT("Test")
	);
	const FString MaterialsDir = FString::Printf(
		TEXT("%s%s/"),
		*PlatformContentDir,
		*AutoMeshDefault->MaterialsDir
	);
	const FString MaterialsSubDir = FString::Printf(
		TEXT("%s/%s/"),
		*MaterialsDir,
		TEXT("Test")
	);
	const FString TexturesDir = FString::Printf(
		TEXT("%s%s/"),
		*PlatformContentDir,
		*AutoMeshDefault->TexturesDir
	);
	const FString TexturesSubDir = FString::Printf(
		TEXT("%s/%s/"),
		*TexturesDir,
		TEXT("Test")
	);
	// StaticMesh
	const FString CubeMeshSrc = FString::Printf(
		TEXT("%s%s"),
		*FPaths::EngineContentDir(),
		TEXT("BasicShapes/Cube.uasset")
	);
	const FString CubeMeshDest = FString::Printf(
		TEXT("%s%s"),
		*MeshesSubDir,
		TEXT("SM_Test_Cube.uasset")
	);
	// Textures
	const FString DiffuseTextureSrc = FString::Printf(
		TEXT("%s%s"),
		*FPaths::EngineContentDir(),
		TEXT("EngineMaterials/DefaultDiffuse.uasset")
	);
	const FString DiffuseTextureDest = FString::Printf(
		TEXT("%s%s"),
		*TexturesSubDir,
		TEXT("T_Test_Cube_D.uasset")
	);
	const FString MaskTextureSrc = FString::Printf(
		TEXT("%s%s"),
		*FPaths::EngineContentDir(),
		TEXT("EngineMaterials/DefaultDiffuse.uasset")
	);
	const FString MaskTextureDest = FString::Printf(
		TEXT("%s%s"),
		*TexturesSubDir,
		TEXT("T_Test_Cube_M.uasset")
	);
	const FString NormalTextureSrc = FString::Printf(
		TEXT("%s%s"),
		*FPaths::EngineContentDir(),
		TEXT("EngineMaterials/DefaultNormal.uasset")
	);
	const FString NormalTextureDest = FString::Printf(
		TEXT("%s%s"),
		*TexturesSubDir,
		TEXT("T_Test_Cube_N.uasset")
	);
	// const FString CubeMeshSrcPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshSrc);
	// const FString CubeMeshDestPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshDest);
	// const FString CubeMeshDestPkgName = FPackageName::FilenameToLongPackageName(*CubeMeshDest);

	TArray<FString> AssetDirs =
	{
		*MeshesSubDir,
		*MaterialsSubDir,
		*TexturesSubDir
	};
	for (FString Dir : AssetDirs)
	{
		if (!FileManager.CreateDirectoryTree(*Dir))
		{
			UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *Dir);
		}
	}

	MockAssetMap.Add(TEXT("CubeMeshDest"), *CubeMeshDest);
	MockAssetMap.Add(TEXT("DiffuseTextureDest"), *DiffuseTextureDest);
	MockAssetMap.Add(TEXT("MaskTextureDest"), *MaskTextureDest);
	MockAssetMap.Add(TEXT("NormalTextureDest"), *NormalTextureDest);
	
	TArray<FString> Src =
	{
		*CubeMeshSrc,
		*DiffuseTextureSrc,
		*MaskTextureSrc,
		*NormalTextureSrc
	};
	TArray<FString> Dest;
	MockAssetMap.GenerateValueArray(Dest);

	for (int32 Index=0; Index != Src.Num(); ++Index)
	{
		if (!FileManager.CopyFile(*Dest[Index], *Src[Index]))
		{
			UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *Src[Index], *Dest[Index])
		}
		else
		{
			FString PkgName = FPackageName::FilenameToLongPackageName(*Dest[Index]);
			UPackage* Pkg = UPackageTools::LoadPackage(*PkgName);
			UObject* Obj = AAutoMesh::GetRenamedObject(Pkg);
			AAutoMesh::SavePackage(Pkg, Obj);
		}
	}
	MockAssetMap.Add(TEXT("PlatformContentDir"), *PlatformContentDir);
	return MockAssetMap;
}