// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/AutoMeshTest.h"

// #include "AudioMixerDevice.h"
#include "AutoMesh.h"
#include "PackageTools.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

BEGIN_DEFINE_SPEC(
	SpecGetAssetMap,
	"Texturematica.AutoMesh.SpecGetAssetMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	UWorld* TestWorld;
	UStaticMesh* CubeMesh;
	UStaticMeshComponent* CubeMeshComponent;
	AStaticMeshActor* CubeMeshActor;
END_DEFINE_SPEC(SpecGetAssetMap)

void SpecGetAssetMap::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
			TestNotNull("TestWorld Exists", TestWorld);
			CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
			TestNotNull("CubeMeshActor Exists", CubeMeshActor);
			CubeMeshComponent = CubeMeshActor->GetStaticMeshComponent();
			TestNotNull("CubeMeshComponent Exists", CubeMeshComponent);
			CubeMesh = Cast<UStaticMesh>(
				StaticLoadObject(
					UStaticMesh::StaticClass(),
					CubeMeshComponent,
					TEXT("/Engine/BasicShapes/Cube.Cube")
				)
			);
			TestNotNull("CubeMesh Exists", CubeMesh);
			CubeMeshComponent->SetStaticMesh(CubeMesh);
		});

		It("should return object / package info of asset", [this]()
		{
			TMap<FString, FString> AssetMap = AAutoMesh::GetAssetMap(CubeMesh);
			TestEqual(TEXT("AssetMap ObjectName"), AssetMap["ObjectName"], TEXT("Cube"));
			TestEqual(TEXT("AssetMap ObjectPath"), AssetMap["ObjectPath"], TEXT("/Engine/BasicShapes/Cube.Cube"));
			TestEqual(TEXT("AssetMap PackageName"), AssetMap["PackageName"], TEXT("/Engine/BasicShapes/Cube"));
			TestEqual(TEXT("AssetMap PackagePath"), AssetMap["PackagePath"], TEXT("/Engine/BasicShapes"));
		});

		It("should return zero-length TMap for invalid Asset", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: Asset"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UStaticMesh* NullMesh = nullptr;
			const TMap<FString, FString> AssetMap = AAutoMesh::GetAssetMap(NullMesh);
			TestEqual(TEXT("Testing Zero-length Map"), AssetMap.Num(), 0);
		});
		
		AfterEach([this]()
		{
			TestWorld = GEngine->GetWorldFromContextObject(CubeMeshActor, EGetWorldErrorMode::ReturnNull);
			TestNotNull("TestWorld Exists", TestWorld);
		});
	});
}

BEGIN_DEFINE_SPEC(
	SpecGetStaticMesh,
	"Texturematica.AutoMesh.SpecGetStaticMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	UWorld* TestWorld;
	UStaticMesh* CubeMesh;
	UStaticMeshComponent* CubeMeshComponent;
	AStaticMeshActor* CubeMeshActor;
END_DEFINE_SPEC(SpecGetStaticMesh)

void SpecGetStaticMesh::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
			TestNotNull("TestWorld Exists", TestWorld);
			CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
			TestNotNull("CubeMeshActor Exists", CubeMeshActor);
			CubeMeshComponent = CubeMeshActor->GetStaticMeshComponent();
			TestNotNull("CubeMeshComponent Exists", CubeMeshComponent);
			CubeMesh = Cast<UStaticMesh>(
				StaticLoadObject(
					UStaticMesh::StaticClass(),
					CubeMeshComponent,
					TEXT("/Engine/BasicShapes/Cube.Cube")
				)
			);
			TestNotNull("CubeMesh Exists", CubeMesh);
			CubeMeshComponent->SetStaticMesh(CubeMesh);
		});

		It("should return valid StaticMesh from StaticMeshActor", [this]()
		{
			const UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMeshActor);
			TestEqual(
				TEXT("Testing StaticMeshActor"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);
		});
		
		It("should return valid StaticMesh from StaticMeshComponent", [this]()
		{
			const UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMeshComponent);
			TestEqual(
				TEXT("Testing StaticMeshComponent"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);	
		});
		
		It("should return valid StaticMesh from StaticMesh", [this]()
		{
			const UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMesh);
			TestEqual(
				TEXT("Testing StaticMesh"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);	
		});
		
		It("should return 'Cube' StaticMesh Name", [this]()
		{
			const UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMesh);
			TestEqual(
				TEXT("Testing SM Object Name"),
				SM->GetName(),
				TEXT("Cube")
			);
		});

		It("should return nullptr from non-StaticMesh object", [this]
		{
			AddExpectedError(
				"Invalid Static Mesh Class",
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			const UStaticMesh* SM = AAutoMesh::GetStaticMesh(TestWorld);
			TestNull(TEXT("Testing null SM"), SM);
		});
		
		AfterEach([this]()
		{
			TestWorld = GEngine->GetWorldFromContextObject(CubeMeshActor, EGetWorldErrorMode::ReturnNull);
			TestNotNull("TestWorld Exists", TestWorld);
			TestWorld->DestroyWorld(false);
		});
	});
}

BEGIN_DEFINE_SPEC(
	SpecGetTexture,
	"Texturematica.AutoMesh.SpecGetTexture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	FString EngineContentDir;
END_DEFINE_SPEC(SpecGetTexture)

void SpecGetTexture::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
			EngineContentDir = FPaths::EngineContentDir();
			EngineContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*EngineContentDir);
		});

		It("should return valid Texture from valid prefix / valid file path", [this]()
		{
			const UTexture* T = AAutoMesh::GetTexture(
				*EngineContentDir,
				TEXT("Engine_MI_Shaders/T_Base_Tile_Diffuse.uasset")
			);
			TestEqual(
				TEXT("Testing Texture Class Name"),
				T->GetClass()->GetName(),
				TEXT("Texture2D")
			);	
		});

		It("should return valid Texture from empty prefix / valid file path", [this]()
		{
			const FString TexturePath = FString::Printf(
				TEXT("%sEngine_MI_Shaders/T_Base_Tile_Diffuse.uasset"),
				*EngineContentDir
			);
			const UTexture* T = AAutoMesh::GetTexture(
				TEXT(""),
				TexturePath
			);
			TestEqual(
				TEXT("Testing Texture Class Name"),
				T->GetClass()->GetName(),
				TEXT("Texture2D")
			);	
		});

		It("should return nullptr from non-Texture asset", [this]()
		{
			AddExpectedError(
				"Failed to find object",
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			const UTexture* T = AAutoMesh::GetTexture(
				*EngineContentDir,
				TEXT("BasicShapes/Cube.uasset")
			);
			TestNull(TEXT("Testing nullptr Texture"), T);
		});
		
		It("should return nullptr from invalid file path", [this]()
		{
			AddExpectedError(
				"Not Exists",
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			const UTexture* T = AAutoMesh::GetTexture(
				*EngineContentDir,
				TEXT("Foo/Bar")
			);
			TestNull(TEXT("Testing Invalid File Path"), T);
		});
	});
}

BEGIN_DEFINE_SPEC(
	SpecCreateAsset,
	"Texturematica.AutoMesh.SpecCreateAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString ContentDir;
	FString PlatformContentDir;
	FString ObjectName;
	FString PackagePath;
	FString PackageName;
END_DEFINE_SPEC(SpecCreateAsset)

void SpecCreateAsset::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			const FString Timestamp = FDateTime::Now().ToString().Replace(TEXT("."), TEXT(""));
			ContentDir = FString::Printf(
				TEXT("%s%s/%s/%s"),
				*FPaths::ProjectPluginsDir(),
				TEXT("Texturematica"),
				TEXT("Content"),
				*Timestamp
			);
			// ContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*ContentDir);
		});

		It("should return valid Material asset", [this]()
		{
			AddExpectedError(
				"Deactivating a context failed when its window couldn't be found",
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UMaterialFactoryNew* TestFactory = NewObject<UMaterialFactoryNew>();
			PackagePath = FString::Printf(
				TEXT("%s/%s"),
				*ContentDir,
				TEXT("Materials")
			);
			ObjectName = TEXT("M_Test");
			PackageName = FString::Printf(
				TEXT("%s/%s"),
				*PackagePath,
				*ObjectName
			);
			UMaterial* NewAsset = Cast<UMaterial>(AAutoMesh::CreateAsset(
				TestFactory,
				UMaterial::StaticClass(),
				ObjectName,
				PackageName,
				PackagePath
			));
			TestNotNull(TEXT("Test Valid Material Asset"), NewAsset);
			TestEqual(
				TEXT("Test Material Class Name"),
				NewAsset->GetClass()->GetName(),
				TEXT("Material")
			);
			TestEqual(
				TEXT("Test Material Name"),
				NewAsset->GetName(),
				TEXT("M_Test")
			);
		});

		It("should return nullptr from invalid param", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: Factory"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UMaterialFactoryNew* TestFactory = nullptr;
			PackagePath = FString::Printf(
				TEXT("%s/%s"),
				*ContentDir,
				TEXT("Materials")
			);
			ObjectName = TEXT("M_Test");
			PackageName = FString::Printf(
				TEXT("%s/%s"),
				*PackagePath,
				*ObjectName
			);
			const UMaterial* NewAsset = Cast<UMaterial>(AAutoMesh::CreateAsset(
				TestFactory,
				UMaterial::StaticClass(),
				ObjectName,
				PackageName,
				PackagePath
			));
			TestNull(TEXT("Test Invalid Param"), NewAsset);
		});
		
		AfterEach([this]()
		{
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				FileManager.DeleteDirectoryRecursively(*PlatformContentDir);
			}
		});
	});
}

BEGIN_DEFINE_SPEC(
	SpecCreateMasterMaterial,
	"Texturematica.AutoMesh.SpecCreateMasterMaterial",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	FString PlatformContentDir;
	UWorld* TestWorld;
	AStaticMeshActor* CubeMeshActor;
	UStaticMeshComponent* CubeMeshComponent;
	UStaticMesh* CubeMesh;
END_DEFINE_SPEC(SpecCreateMasterMaterial)

void SpecCreateMasterMaterial::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			TMap<FString, FString>MockAssetMap = AAutoMesh::GetMockAssetMap();
			PlatformContentDir = MockAssetMap["PlatformContentDir"];
			/*
			const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
			// Build out directory tree for test files
			PlatformContentDir = FString::Printf(
				TEXT("%s%s/%s/"),
				*FPaths::ProjectPluginsDir(),
				TEXT("Texturematica"),
				TEXT("Content")
			);
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
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
			const FString CubeMeshSrcPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshSrc);
			const FString CubeMeshDestPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshDest);
			const FString CubeMeshDestPkgName = FPackageName::FilenameToLongPackageName(*CubeMeshDest);
			UE_LOG(LogAutoMesh, Warning, TEXT("CubeMeshSrcPlatform: %s"), *CubeMeshSrcPlatform);
			UE_LOG(LogAutoMesh, Warning, TEXT("CubeMeshDestPlatform: %s"), *CubeMeshDestPlatform);
			UE_LOG(LogAutoMesh, Warning, TEXT("CubeMeshDestPkgName: %s"), *CubeMeshDestPkgName);

			if (!FileManager.CreateDirectoryTree(*MeshesSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MeshesSubDir);
			}
			if (!FileManager.CreateDirectoryTree(*MaterialsSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MaterialsSubDir);
			}
			if (!FileManager.CopyFile(*CubeMeshDest, *CubeMeshSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *CubeMeshSrc, *CubeMeshDest)
			}
			*/

			// Create Test World for plugin Content assets
			TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
			TestNotNull("TestWorld Exists", TestWorld);
			CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
			TestNotNull("CubeMeshActor Exists", CubeMeshActor);
			CubeMeshComponent = CubeMeshActor->GetStaticMeshComponent();
			TestNotNull("CubeMeshComponent Exists", CubeMeshComponent);

			// Get object from package
			UPackage* CubeMeshPkg = UPackageTools::LoadPackage(MockAssetMap["CubeMeshDest"]);
			/*
			CubeMeshPkg->FullyLoad();
			TArray<UObject*> Objs;
			TArray<UPackage*> Pkgs;
			Pkgs.Add(CubeMeshPkg);
			UPackageTools::GetObjectsInPackages(&Pkgs, Objs);
			UObject* CubeMeshObj = Objs[0];
			UE_LOG(LogAutoMesh, Warning, TEXT("CubeMeshObj FullName: %s"), *CubeMeshObj->GetFullName())
			CubeMeshObj->Rename(*FPackageName::GetShortName(*CubeMeshDestPkgName));  // Rename object to match package
			*/
			CubeMesh = Cast<UStaticMesh>(
				LoadObject<UStaticMesh>(
					CubeMeshPkg,
					*FPackageName::GetShortName(*CubeMeshPkg->GetName())
				)
			);
			TestNotNull("CubeMesh Exists", CubeMesh);
			CubeMeshComponent->SetStaticMesh(CubeMesh);
		});

		It("should return valid MasterMaterial for valid StaticMesh", [this]()
		{
			AddExpectedError(
				TEXT("Deactivating a context failed when its window couldn't be found"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UMaterial* MasterMaterial = AAutoMesh::CreateMasterMaterial(CubeMesh);
			TestNotNull(TEXT("Testing Valid Material"), MasterMaterial);
			TestEqual(
				TEXT("Testing Material Class Name"),
				MasterMaterial->GetClass()->GetName(),
				TEXT("Material")
			);
			TestEqual(
				TEXT("Testing Material Name"),
				MasterMaterial->GetName(),
				TEXT("M_Test")
			);
			TestEqual(
				TEXT("Testing Material Expression Total"),
				MasterMaterial->Expressions.Num(),
				3
			);

			for (UMaterialExpression* Expr : MasterMaterial->Expressions)
			{
				UMaterialExpressionTextureSampleParameter2D* ExprTexture2D =
					Cast<UMaterialExpressionTextureSampleParameter2D>(Expr);
				TestNotNull(TEXT("Testing Valid Texture2D MaterialExpression"), ExprTexture2D);
				TestTrue(TEXT("Testing MaterialExpression Param Name"),
					(
					ExprTexture2D->GetParameterName() == FName(TEXT("Diffuse")) ||
					ExprTexture2D->GetParameterName() == FName(TEXT("Mask")) ||
					ExprTexture2D->GetParameterName() == FName(TEXT("Normal"))
					)
				);
			}
		});

		It("should return nullptr from invalid StaticMesh", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: StaticMesh"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UStaticMesh* NullMesh = nullptr;
			UMaterial* MasterMaterial = AAutoMesh::CreateMasterMaterial(NullMesh);
			TestNull(TEXT("Testing Invalid StaticMesh"), MasterMaterial);
		});

		AfterEach([this]()
		{
			TestWorld = GEngine->GetWorldFromContextObject(CubeMeshActor, EGetWorldErrorMode::ReturnNull);
			TestNotNull("TestWorld Exists", TestWorld);
			TestWorld->DestroyWorld(false);
			
			IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				FileManager.DeleteDirectoryRecursively(*PlatformContentDir);
			}
		});
	});
}
BEGIN_DEFINE_SPEC(
	SpecCreateMaterialInstance,
	"Texturematica.AutoMesh.SpecCreateMaterialInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString PlatformContentDir;
	UWorld* TestWorld;
	AStaticMeshActor* CubeMeshActor;
	UStaticMeshComponent* CubeMeshComponent;
	UStaticMesh* CubeMesh;
	UMaterial* MasterMaterial;
	UMaterialInstanceConstant* MaterialInstance;
END_DEFINE_SPEC(SpecCreateMaterialInstance)

void SpecCreateMaterialInstance::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
			// Build out directory tree for test files
			PlatformContentDir = FString::Printf(
				TEXT("%s%s/%s/"),
				*FPaths::ProjectPluginsDir(),
				TEXT("Texturematica"),
				TEXT("Content")
			);
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
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
			const FString CubeMeshSrcPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshSrc);
			const FString CubeMeshDestPlatform = FileManager.ConvertToAbsolutePathForExternalAppForRead(*CubeMeshDest);
			const FString CubeMeshDestPkgName = FPackageName::FilenameToLongPackageName(*CubeMeshDest);

			if (!FileManager.CreateDirectoryTree(*MeshesSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MeshesSubDir);
			}
			if (!FileManager.CreateDirectoryTree(*MaterialsSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MaterialsSubDir);
			}
			if (!FileManager.CreateDirectoryTree(*TexturesSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *TexturesSubDir);
			}
			if (!FileManager.CopyFile(*CubeMeshDest, *CubeMeshSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *CubeMeshSrc, *CubeMeshDest)
			}
			if (!FileManager.CopyFile(*DiffuseTextureDest, *DiffuseTextureSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *DiffuseTextureSrc, *CubeMeshDest)
			}
			if (!FileManager.CopyFile(*MaskTextureDest, *MaskTextureSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *MaskTextureSrc, *CubeMeshDest)
			}
			if (!FileManager.CopyFile(*NormalTextureDest, *NormalTextureSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *NormalTextureSrc, *CubeMeshDest)
			}
			
			// Create Test World for plugin Content assets
			TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
			TestNotNull("TestWorld Exists", TestWorld);
			CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
			TestNotNull("CubeMeshActor Exists", CubeMeshActor);
			CubeMeshComponent = CubeMeshActor->GetStaticMeshComponent();
			TestNotNull("CubeMeshComponent Exists", CubeMeshComponent);

			// Get object from package
			UPackage* CubeMeshPkg = UPackageTools::LoadPackage(*CubeMeshDestPkgName);
			UObject* CubeMeshObj = AAutoMesh::GetRenamedObject(CubeMeshPkg);
			/*
			CubeMeshPkg->FullyLoad();
			TArray<UObject*> Objs;
			TArray<UPackage*> Pkgs;
			Pkgs.Add(CubeMeshPkg);
			UPackageTools::GetObjectsInPackages(&Pkgs, Objs);
			UObject* CubeMeshObj = Objs[0];
			
			CubeMeshObj->Rename(*FPackageName::GetShortName(*CubeMeshDestPkgName));  // Rename to match dest package
			*/
			UE_LOG(LogAutoMesh, Warning, TEXT("CubeMeshObj FullName: %s"), *CubeMeshObj->GetFullName())
			CubeMesh = Cast<UStaticMesh>(
				LoadObject<UStaticMesh>(
					CubeMeshPkg,
					*CubeMeshObj->GetName()
				)
			);
			TestNotNull("CubeMesh Exists", CubeMesh);
			CubeMeshComponent->SetStaticMesh(CubeMesh);
		});

		It("should return valid MaterialInstance from valid MasterMaterial and StaticMesh", [this]()
		{
			MasterMaterial = AAutoMesh::CreateMasterMaterial(CubeMesh);
			TestNotNull(TEXT("Testing Valid MasterMaterial"), MasterMaterial);
			MaterialInstance = AAutoMesh::CreateMaterialInstance(MasterMaterial, CubeMesh);
			TestNotNull(TEXT("Testing Valid Material Instance"), MaterialInstance);
			TestEqual(
				TEXT("Testing Material Instance Class"),
				MaterialInstance->GetClass()->GetName(),
				TEXT("MaterialInstanceConstant")
			);
			TestEqual(
				TEXT("Testing Material Instance Name"),
				MaterialInstance->GetName(),
				TEXT("MI_Test_Cube")
			);
		});

		It("should return nullptr for invalid MasterMaterial and valid StaticMesh", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: MasterMaterial"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UMaterial* NullMaterial = nullptr;
			MaterialInstance = AAutoMesh::CreateMaterialInstance(NullMaterial, CubeMesh);
			TestNull(TEXT("Testing Invalid MasterMaterial"), MaterialInstance);
		});

		It("should return nullptr for valid MasterMaterial and invalid StaticMesh", [this]()
		{
			AddExpectedError(
				TEXT("Deactivating a context failed when its window couldn't be found"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			AddExpectedError(
				TEXT("nullptr: StaticMesh"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UStaticMesh* NullMesh = nullptr;
			MasterMaterial = AAutoMesh::CreateMasterMaterial(CubeMesh);
			TestNotNull(TEXT("Testing Valid MasterMaterial"), MasterMaterial);
			MaterialInstance = AAutoMesh::CreateMaterialInstance(MasterMaterial, NullMesh);
			TestNull(TEXT("Testing Invalid StaticMesh"), MaterialInstance);
		});
		
		AfterEach([this]()
		{
			TestWorld = GEngine->GetWorldFromContextObject(CubeMeshActor, EGetWorldErrorMode::ReturnNull);
			TestNotNull("TestWorld Exists", TestWorld);
			TestWorld->DestroyWorld(false);

			TArray<UObject*> ObjsToDelete =
			{
				MaterialInstance,
				MasterMaterial
			};

			for (UObject* Obj : ObjsToDelete)
			{
				if (IsValid(Obj))
				{
					UE_LOG(LogAutoMesh, Warning, TEXT("Destroying Object: %s"), *Obj->GetFullName());
					Obj->ConditionalBeginDestroy();
				}
			}
			
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				UE_LOG(LogAutoMesh, Warning, TEXT("Deleting Dir: %s"), *PlatformContentDir);
				if (!FileManager.DeleteDirectoryRecursively(*PlatformContentDir))
				{
					UE_LOG(LogAutoMesh, Warning, TEXT("DeleteDirectoryRecursively Fail: %s"), *PlatformContentDir)
				}
			}
		});
	});
}
			
BEGIN_DEFINE_SPEC(
	SpecGetRenamedObject,
	"Texturematica.AutoMesh.SpecGetRenamedObject",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString PlatformContentDir;
	FString CubeMeshDestPkgName;
END_DEFINE_SPEC(SpecGetRenamedObject)

void SpecGetRenamedObject::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
			// Build out directory tree for test files
			PlatformContentDir = FString::Printf(
				TEXT("%s%s/%s/"),
				*FPaths::ProjectPluginsDir(),
				TEXT("Texturematica"),
				TEXT("Content")
			);
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
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
			CubeMeshDestPkgName = FPackageName::FilenameToLongPackageName(*CubeMeshDest);

			if (!FileManager.CreateDirectoryTree(*MeshesSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MeshesSubDir);
			}
			if (!FileManager.CopyFile(*CubeMeshDest, *CubeMeshSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *CubeMeshSrc, *CubeMeshDest)
			}
		});

		It("should return Object name identical to Package name", [this]()
		{
			UPackage* CubeMeshPkg = UPackageTools::LoadPackage(*CubeMeshDestPkgName);
			UObject* CubeMeshObj = AAutoMesh::GetRenamedObject(CubeMeshPkg);
			TestNotNull(TEXT("Testing Valid Object"), CubeMeshObj);
			TestEqual(TEXT("Test Object Name"), CubeMeshObj->GetName(), TEXT("SM_Test_Cube"));
		}
		);

		It("should return nullptr for invalid Package", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: Package"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);			
			UPackage* CubeMeshPkg = nullptr;
			UObject* CubeMeshObj = AAutoMesh::GetRenamedObject(CubeMeshPkg);
			TestNull(TEXT("Test Invalid Package"), CubeMeshObj);
		});
		
		AfterEach([this]()
		{
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				UE_LOG(LogAutoMesh, Warning, TEXT("Deleting Dir: %s"), *PlatformContentDir);
				if (!FileManager.DeleteDirectoryRecursively(*PlatformContentDir))
				{
					UE_LOG(LogAutoMesh, Warning, TEXT("DeleteDirectoryRecursively Fail: %s"), *PlatformContentDir)
				}
			}			
		});
	});
}

BEGIN_DEFINE_SPEC(
	SpecSavePackage,
	"Texturematica.AutoMesh.SpecSavePackage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString PlatformContentDir;
	FString CubeMeshDestPkgName;
END_DEFINE_SPEC(SpecSavePackage)

void SpecSavePackage::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			const AAutoMesh* AutoMeshDefault = GetDefault<AAutoMesh>(AAutoMesh::StaticClass());  // CDO
			// Build out directory tree for test files
			PlatformContentDir = FString::Printf(
				TEXT("%s%s/%s/"),
				*FPaths::ProjectPluginsDir(),
				TEXT("Texturematica"),
				TEXT("Content")
			);
			PlatformContentDir = FileManager.ConvertToAbsolutePathForExternalAppForRead(*PlatformContentDir);
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
			CubeMeshDestPkgName = FPackageName::FilenameToLongPackageName(*CubeMeshDest);

			if (!FileManager.CreateDirectoryTree(*MeshesSubDir))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CreateDirectoryTree Failure: %s"), *MeshesSubDir);
			}
			if (!FileManager.CopyFile(*CubeMeshDest, *CubeMeshSrc))
			{
				UE_LOG(LogAutoMesh, Error, TEXT("CopyFile Failure. SRC: %s, DEST: %s"), *CubeMeshSrc, *CubeMeshDest)
			}
		});

		It("should return true for valid Package, valid Object", [this]()
		{
			UPackage* CubeMeshPkg = UPackageTools::LoadPackage(*CubeMeshDestPkgName);
			UObject* CubeMeshObj = AAutoMesh::GetRenamedObject(CubeMeshPkg);
			TestTrue(TEXT("Test Successful Save"), AAutoMesh::SavePackage(CubeMeshPkg, CubeMeshObj));
		});

		It("should return false for invalid Package, valid Object", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: Package"),
				EAutomationExpectedErrorFlags::Contains,
				2
			);
			AddExpectedError(
				TEXT("Package Save Failure"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UPackage* CubeMeshPkg = nullptr;
			UObject* CubeMeshObj = AAutoMesh::GetRenamedObject(CubeMeshPkg);
			TestFalse(TEXT("Test Save Failure"), AAutoMesh::SavePackage(CubeMeshPkg, CubeMeshObj));
		});
		
		It("should return false for valid Package, invalid Object", [this]()
		{
			AddExpectedError(
				TEXT("nullptr: Object"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			AddExpectedError(
				TEXT("Package Save Failure"),
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			UPackage* CubeMeshPkg = UPackageTools::LoadPackage(*CubeMeshDestPkgName);
			UPackage* CubeMeshObj = nullptr;
			TestFalse(TEXT("Test Save Failure"), AAutoMesh::SavePackage(CubeMeshPkg, CubeMeshObj));
		});
		
		AfterEach([this]()
		{
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				UE_LOG(LogAutoMesh, Warning, TEXT("Deleting Dir: %s"), *PlatformContentDir);
				if (!FileManager.DeleteDirectoryRecursively(*PlatformContentDir))
				{
					UE_LOG(LogAutoMesh, Warning, TEXT("DeleteDirectoryRecursively Fail: %s"), *PlatformContentDir)
				}
			}			
		});		
	});
}

BEGIN_DEFINE_SPEC(
	SpecGetMockAssetMap,
	"Texturematica.AutoMesh.SpecGetMockAssetMap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	FString PlatformContentDir;
END_DEFINE_SPEC(SpecGetMockAssetMap)

void SpecGetMockAssetMap::Define()
{
	Describe("Execute", [this]()
	{
		BeforeEach([this]()
		{
			// 
		});

		It("should return valid MockAssetMap", [this]()
		{
			TMap<FString, FString> MockAssetMap = AAutoMesh::GetMockAssetMap();
			TestEqual(TEXT("Test MockAsset Count"), MockAssetMap.Num(), 5);
			TestTrue(TEXT("Test Mesh Key"), MockAssetMap.Contains(TEXT("CubeMeshDest")));
			TestTrue(TEXT("Test Diffuse Key"), MockAssetMap.Contains(TEXT("DiffuseTextureDest")));
			TestTrue(TEXT("Test Mask Key"), MockAssetMap.Contains(TEXT("MaskTextureDest")));
			TestTrue(TEXT("Test Normal Key"), MockAssetMap.Contains(TEXT("NormalTextureDest")));
			TestTrue(TEXT("Test ContentDir Key"), MockAssetMap.Contains(TEXT("PlatformContentDir")));
			for (auto& Elem : MockAssetMap)
			{
				TestNotNull(TEXT("Test Valid Asset String"), *Elem.Value);
			}
			PlatformContentDir = MockAssetMap["PlatformContentDir"];
		});

		AfterEach([this]()
		{
			if (FileManager.DirectoryExists(*PlatformContentDir))
			{
				UE_LOG(LogAutoMesh, Warning, TEXT("Deleting Dir: %s"), *PlatformContentDir);
				if (!FileManager.DeleteDirectoryRecursively(*PlatformContentDir))
				{
					UE_LOG(LogAutoMesh, Warning, TEXT("DeleteDirectoryRecursively Fail: %s"), *PlatformContentDir)
				}
			}
		});
	});
}