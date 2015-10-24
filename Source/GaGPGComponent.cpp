#include "GaGPGComponent.h"

#include "System/Scene/ScnEntity.h"

#if PLATFORM_ANDROID
#include <android_native_app_glue.h>

#include "gpg/android_initialization.h"

#endif

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( GaGPGComponent );

void GaGPGComponent::StaticRegisterClass()
{
	ReRegisterClass< GaGPGComponent, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
GaGPGComponent::GaGPGComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaGPGComponent::~GaGPGComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// openLeaderboards
void GaGPGComponent::openLeaderboards()
{
#if PLATFORM_ANDROID
	PSY_LOG( "INFO: GaGPGComponent::openLeaderboards!" );
	if( GameServices_ && !GameServices_->IsAuthorized() )
	{
		GameServices_->StartAuthorizationUI();
	}

	if( GameServices_ )
	{
		PSY_LOG( "INFO: ShowAllUI!" );
		GameServices_->Leaderboards().ShowAllUIBlocking();
#if 0
		GameServices_->Leaderboards().ShowAllUI(
			[]( UIStatus const & )
			{
				// TODO: Handle status.
			} );
#endif
}
#else
	PSY_LOG( "ERROR: GaGPGComponent::openLeaderboards unimplemented!" );
#endif
}

//////////////////////////////////////////////////////////////////////////
// openAchievements
void GaGPGComponent::openAchievements()
{
#if PLATFORM_ANDROID
	PSY_LOG( "INFO: GaGPGComponent::openAchievements!" );
	if( GameServices_ && !GameServices_->IsAuthorized() )
	{
		GameServices_->StartAuthorizationUI();
	}

	if( GameServices_ )
	{
		PSY_LOG( "INFO: ShowAllUI!" );
		GameServices_->Achievements().ShowAllUIBlocking();
#if 0
		GameServices_->Achievements().ShowAllUI(
			[]( UIStatus const & )
			{
				// TODO: Handle status.
			} );
#endif
	}
#else
	PSY_LOG( "ERROR: GaGPGComponent::openAchievements unimplemented!" );
#endif
}

//////////////////////////////////////////////////////////////////////////
// submitScore
void GaGPGComponent::submitScore( const char* ID, BcS64 Score )
{
#if PLATFORM_ANDROID
	PSY_LOG( "INFO: GaGPGComponent::submitScore!" );
	if( GameServices_ )
	{
		GameServices_->Leaderboards().SubmitScore( ID, static_cast< BcU64 >( Score ) );
	}
#else
	PSY_LOG( "ERROR: GaGPGComponent::submitScore unimplemented!" );
#endif
}

//////////////////////////////////////////////////////////////////////////
// unlockAchievement
void GaGPGComponent::unlockAchievement( const char* ID )
{
#if PLATFORM_ANDROID
	PSY_LOG( "INFO: GaGPGComponent::unlockAchievement!" );
	if( GameServices_ )
	{
		GameServices_->Achievements().Unlock( ID );
	}
#else
	PSY_LOG( "ERROR: GaGPGComponent::unlockAchievement unimplemented!" );
#endif
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void GaGPGComponent::onAttach( ScnEntityWeakRef Parent )
{
#if PLATFORM_ANDROID
	extern struct android_app* GAndroidApp;

	gpg::AndroidInitialization::android_main( GAndroidApp );

	gpg::PlatformConfiguration PlatformConfig;
	PlatformConfig.SetActivity( GAndroidApp->activity->clazz );

	//gpg::GameServices::Builder::OnAuthActionStartedCallback started_callback,
	//gpg::GameServices::Builder::OnAuthActionFinishedCallback
	GameServices_ = gpg::GameServices::Builder()
		.SetLogging( gpg::DEFAULT_ON_LOG, gpg::LogLevel::VERBOSE )
		.SetOnAuthActionStarted(
			[]( gpg::AuthOperation op )
			{
				PSY_LOG( "Authentication started" );
				//is_auth_in_progress_ = true;
			} )
		.SetOnAuthActionFinished(
			[]( gpg::AuthOperation op,
				gpg::AuthStatus status )
			{
				PSY_LOG( "Authentication finished" );
				//LOGI("Fetching all blocking");
				//gpg::AchievementManager::FetchAllResponse fetchResponse = game_services_->Achievements().FetchAllBlocking(std::chrono::milliseconds(1000));
				//LOGI("--------------------------------------------------------------");	
				//LOGI("Fetching all nonblocking");
				//game_services_->Achievements().FetchAll(gpg::DataSource::CACHE_OR_NETWORK, [] (gpg::AchievementManager::FetchAllResponse response) {LOGI("Achievement response status: %d", response.status);});
				//LOGI("--------------------------------------------------------------");
			} )
		.Create( PlatformConfig );
	if( GameServices_ && !GameServices_->IsAuthorized() )
	{
		GameServices_->StartAuthorizationUI();
	}
#endif

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void GaGPGComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}
