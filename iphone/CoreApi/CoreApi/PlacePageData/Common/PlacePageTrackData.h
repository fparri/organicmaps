#import <Foundation/Foundation.h>
#import "MWMTypes.h"

@class ElevationProfileData;

NS_ASSUME_NONNULL_BEGIN

@interface PlacePageTrackData : NSObject

@property(nonatomic, readonly) MWMTrackID trackId;
@property(nonatomic, readonly) MWMMarkGroupID groupId;
@property(nonatomic, readonly, nullable) ElevationProfileData * elevationProfileData;

@end

NS_ASSUME_NONNULL_END
