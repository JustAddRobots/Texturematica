// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/AutoMeshTest.h"

#include "AutoMesh.h"
#include "Engine/StaticMeshActor.h"
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
	Describe("Execute()", [this]
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

		It("should return object and package info of asset", [this]
		{
			TMap<FString, FString> AssetMap = AAutoMesh::GetAssetMap(CubeMesh);
			TestEqual(TEXT("AssetMap ObjectName"), AssetMap["ObjectName"], TEXT("Cube"));
			TestEqual(TEXT("AssetMap ObjectPath"), AssetMap["ObjectPath"], TEXT("/Engine/BasicShapes/Cube.Cube"));
			TestEqual(TEXT("AssetMap PackageName"), AssetMap["PackageName"], TEXT("/Engine/BasicShapes/Cube"));
			TestEqual(TEXT("AssetMap PackagePath"), AssetMap["PackagePath"], TEXT("/Engine/BasicShapes"));
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

		It("should return valid Texture from valid prefix and valid file path", [this]()
		{
			const UTexture* T = AAutoMesh::GetTexture(
				*EngineContentDir,
				TEXT("Engine_MI_Shaders/T_Base_Tile_Diffuse.uasset")
			);
			TestEqual(
				TEXT("Testing Texture"),
				T->GetClass()->GetName(),
				TEXT("Texture")
			);	
		});

		It("should return valid Texture from empty prefix and valid file path", [this]()
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
				TEXT("Testing Texture"),
				T->GetClass()->GetName(),
				TEXT("Texture")
			);	
		});

		It("should return nullptr from non-Texture asset", [this]()
		{
			AddExpectedError(
				"Invalid Texture",
				EAutomationExpectedErrorFlags::Contains,
				1
			);
			const UTexture* T = AAutoMesh::GetTexture(
				*EngineContentDir,
				TEXT("%sBasicShapes/Cube.Cube")
			);
			TestNull(TEXT("Testing null T"), T);
		});
	});
}
