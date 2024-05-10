package app.organicmaps.routing;

import androidx.annotation.NonNull;

import app.organicmaps.settings.RoadType;
import app.organicmaps.settings.RoutingStrategyType;

import java.util.HashSet;
import java.util.Set;

public class RoutingOptions
{
  public static void addOption(@NonNull RoadType roadType)
  {
    nativeAddOption(roadType.ordinal());
  }

  public static void removeOption(@NonNull RoadType roadType)
  {
    nativeRemoveOption(roadType.ordinal());
  }

  public static boolean hasOption(@NonNull RoadType roadType)
  {
    return nativeHasOption(roadType.ordinal());
  }

  public static int getStrategy()
  {
    return nativeGetStrategy();
  }

  public static void setStrategy(@NonNull RoutingStrategyType routingStrategyType)
  {
    nativeSetStrategy(routingStrategyType.ordinal());
  }

  private static native void nativeAddOption(int option);
  private static native void nativeRemoveOption(int option);
  private static native boolean nativeHasOption(int option);
  private static native int nativeGetStrategy();
  private static native void nativeSetStrategy(int strategy);

  public static boolean hasAnyOptions()
  {
    for (RoadType each : RoadType.values())
    {
      if (hasOption(each))
        return true;
    }
    return false;
  }

  @NonNull
  public static Set<RoadType> getActiveRoadTypes()
  {
    Set<RoadType> roadTypes = new HashSet<>();
    for (RoadType each : RoadType.values())
    {
      if (hasOption(each))
        roadTypes.add(each);
    }
    return roadTypes;
  }

  public static RoutingStrategyType getActiveRoutingStrategyType()
  {
    int strategyType = getStrategy();
    return RoutingStrategyType.values()[strategyType];
  }
}
