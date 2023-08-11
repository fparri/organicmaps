#include "testing/testing.hpp"

#include "types_helper.hpp"

#include "coding/string_utf8_multilang.hpp"

#include "generator/feature_builder.hpp"
#include "generator/generator_tests_support/test_with_classificator.hpp"
#include "generator/geometry_holder.hpp"

#include "indexer/data_header.hpp"
#include "indexer/feature_visibility.hpp"
#include "indexer/ftypes_matcher.hpp"

#include "base/geo_object_id.hpp"

#include <limits>

namespace feature_builder_test
{
using namespace feature;
using namespace generator::tests_support;
using namespace tests;

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_ManyTypes)
{
  FeatureBuilder fb1;
  FeatureBuilderParams params;

  char const * arr1[][1] = {
    { "building" },
  };
  AddTypes(params, arr1);

  char const * arr2[][2] = {
    { "place", "country" },
    { "place", "state" },
    /// @todo Can't realize is it deprecated or we forgot to add clear styles for it.
    //{ "place", "county" },
    { "place", "region" },
    { "place", "city" },
    { "place", "town" },
  };
  AddTypes(params, arr2);

  params.FinishAddingTypes();
  params.AddHouseNumber("75");
  params.AddHouseName("Best House");
  params.AddName("default", "Name");

  fb1.SetParams(params);
  fb1.SetCenter(m2::PointD(0, 0));

  TEST(fb1.RemoveInvalidTypes(), ());
  TEST(fb1.IsValid(), (fb1));

  FeatureBuilder::Buffer buffer;
  TEST(fb1.PreSerializeAndRemoveUselessNamesForIntermediate(), ());
  fb1.SerializeForIntermediate(buffer);

  FeatureBuilder fb2;
  fb2.DeserializeFromIntermediate(buffer);

  TEST(fb2.IsValid(), (fb2));
  TEST_EQUAL(fb1, fb2, ());
  TEST_EQUAL(fb2.GetTypesCount(), 6, ());
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_LineTypes)
{
  FeatureBuilder fb1;
  FeatureBuilderParams params;

  char const * arr2[][2] = {
    { "railway", "rail" },
    { "highway", "motorway" },
    { "hwtag", "oneway" },
    { "psurface", "paved_good" },
    { "junction", "roundabout" },
  };

  AddTypes(params, arr2);
  params.FinishAddingTypes();
  fb1.SetParams(params);

  fb1.AddPoint(m2::PointD(0, 0));
  fb1.AddPoint(m2::PointD(1, 1));
  fb1.SetLinear();

  TEST(fb1.RemoveInvalidTypes(), ());
  TEST(fb1.IsValid(), (fb1));

  FeatureBuilder::Buffer buffer;
  TEST(fb1.PreSerializeAndRemoveUselessNamesForIntermediate(), ());
  fb1.SerializeForIntermediate(buffer);

  FeatureBuilder fb2;
  fb2.DeserializeFromIntermediate(buffer);

  TEST(fb2.IsValid(), (fb2));
  TEST_EQUAL(fb1, fb2, ());

  TEST_EQUAL(fb2.GetTypesCount(), 5, ());
  ftypes::IsRoundAboutChecker::Instance()(fb2.GetTypes());
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_Waterfall)
{
  FeatureBuilder fb1;
  FeatureBuilderParams params;

  char const * arr[][2] = {{"waterway", "waterfall"}};
  AddTypes(params, arr);
  TEST(params.FinishAddingTypes(), ());

  fb1.SetParams(params);
  fb1.SetCenter(m2::PointD(1, 1));

  TEST(fb1.RemoveInvalidTypes(), ());
  TEST(fb1.IsValid(), (fb1));

  FeatureBuilder::Buffer buffer;
  TEST(fb1.PreSerializeAndRemoveUselessNamesForIntermediate(), ());
  fb1.SerializeForIntermediate(buffer);

  FeatureBuilder fb2;
  fb2.DeserializeFromIntermediate(buffer);

  TEST(fb2.IsValid(), (fb2));
  TEST_EQUAL(fb1, fb2, ());
  TEST_EQUAL(fb2.GetTypesCount(), 1, ());
}

UNIT_CLASS_TEST(TestWithClassificator, FBbuilder_GetMostGeneralOsmId)
{
  FeatureBuilder fb;

  fb.AddOsmId(base::MakeOsmNode(1));
  TEST_EQUAL(fb.GetMostGenericOsmId(), base::MakeOsmNode(1), ());

  fb.AddOsmId(base::MakeOsmNode(2));
  fb.AddOsmId(base::MakeOsmWay(1));
  TEST_EQUAL(fb.GetMostGenericOsmId(), base::MakeOsmWay(1), ());

  fb.AddOsmId(base::MakeOsmNode(3));
  fb.AddOsmId(base::MakeOsmWay(2));
  fb.AddOsmId(base::MakeOsmRelation(1));
  TEST_EQUAL(fb.GetMostGenericOsmId(), base::MakeOsmRelation(1), ());
}

UNIT_CLASS_TEST(TestWithClassificator, FVisibility_RemoveUselessTypes)
{
  Classificator const & c = classif();

  {
    std::vector<uint32_t> types;
    types.push_back(c.GetTypeByPath({ "building" }));
    types.push_back(c.GetTypeByPath({ "amenity", "theatre" }));

    TEST(RemoveUselessTypes(types, GeomType::Area), ());
    TEST_EQUAL(types.size(), 2, ());
  }

  {
    std::vector<uint32_t> types;
    types.push_back(c.GetTypeByPath({ "highway", "primary" }));
    types.push_back(c.GetTypeByPath({ "building" }));

    TEST(RemoveUselessTypes(types, GeomType::Area, true /* emptyName */), ());
    TEST_EQUAL(types.size(), 1, ());
    TEST_EQUAL(types[0], c.GetTypeByPath({ "building" }), ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_RemoveUselessNames)
{
  FeatureBuilderParams params;

  char const * arr3[][3] = { { "boundary", "administrative", "2" } };
  AddTypes(params, arr3);
  char const * arr2[][2] = { { "barrier", "fence" } };
  AddTypes(params, arr2);
  params.FinishAddingTypes();

  params.AddName("default", "Name");
  params.AddName("ru", "Имя");

  FeatureBuilder fb1;
  fb1.SetParams(params);

  fb1.AddPoint(m2::PointD(0, 0));
  fb1.AddPoint(m2::PointD(1, 1));
  fb1.SetLinear();

  TEST(!fb1.GetName(0).empty(), ());
  TEST(!fb1.GetName(8).empty(), ());

  fb1.RemoveUselessNames();

  TEST(fb1.GetName(0).empty(), ());
  TEST(fb1.GetName(8).empty(), ());

  TEST(fb1.IsValid(), (fb1));
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_ParamsParsing)
{
  FeatureBuilderParams params;

  params.MakeZero();
  TEST(params.AddHouseNumber("123"), ());
  TEST_EQUAL(params.house.Get(), "123", ());

  params.MakeZero();
  TEST(params.AddHouseNumber("0000123"), ());
  TEST_EQUAL(params.house.Get(), "123", ());

  params.MakeZero();
  TEST(params.AddHouseNumber("000000"), ());
  TEST_EQUAL(params.house.Get(), "0", ());
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_Housenumbers)
{
  FeatureBuilderParams params;
  std::string_view name;

  params.MakeZero();
  name = {};
  params.AddHouseName("75a"); // addr:housenumber
  params.AddHouseName("Sandpiper"); // addr:housename
  TEST_EQUAL(params.house.Get(), "75a", ());
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), ());
  TEST_EQUAL(name, "Sandpiper", ());

  params.MakeZero();
  name = {};
  params.AddHouseName("75"); // addr:housenumber
  params.AddHouseName("Bld 2"); // addr:housename
  TEST_EQUAL(params.house.Get(), "75", ());
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), ());
  TEST_EQUAL(name, "Bld 2", ());

  params.MakeZero();
  name = {};
  params.AddHouseName("75"); // addr:housenumber
  params.AddHouseName("2"); // addr:housename
  TEST_EQUAL(params.house.Get(), "2", ());
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), ());
  TEST_EQUAL(name, "75", ());

  params.MakeZero();
  name = {};
  params.name.AddString(StringUtf8Multilang::kDefaultCode, "The Mansion");
  params.AddHouseName("75a"); // addr:housenumber
  params.AddHouseName("Sandpiper"); // addr:housename
  TEST_EQUAL(params.house.Get(), "75a", ());
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), ());
  TEST_EQUAL(name, "The Mansion", ());

  params.MakeZero();
  name = {};
  params.AddHouseName("75a"); // addr:housenumber
  params.AddHouseName("2"); // addr:housename
  params.name.AddString(StringUtf8Multilang::kDefaultCode, "The Mansion");
  TEST_EQUAL(params.house.Get(), "2", ());
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), ());
  TEST_EQUAL(name, "The Mansion", ());
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_SerializeLocalityObjectForBuildingPoint)
{
  FeatureBuilder fb;
  FeatureBuilderParams params;

  char const * arr1[][1] = {
    { "building" },
  };
  AddTypes(params, arr1);

  params.FinishAddingTypes();
  params.AddHouseNumber("75");
  params.AddHouseName("Best House");
  params.AddName("default", "Name");

  fb.AddOsmId(base::MakeOsmNode(1));
  fb.SetParams(params);
  fb.SetCenter(m2::PointD(10.1, 15.8));

  TEST(fb.RemoveInvalidTypes(), ());
  TEST(fb.IsValid(), (fb));

  feature::DataHeader header;
  header.SetGeometryCodingParams(serial::GeometryCodingParams());
  header.SetScales({scales::GetUpperScale()});
  feature::GeometryHolder holder(fb, header);

  auto & buffer = holder.GetBuffer();
  TEST(fb.PreSerializeAndRemoveUselessNamesForMwm(buffer), ());
  fb.SerializeLocalityObject(serial::GeometryCodingParams(), buffer);
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_SerializeAccuratelyForIntermediate)
{
  FeatureBuilder fb1;
  FeatureBuilderParams params;

  char const * arr2[][2] = {
    { "railway", "rail" },
    { "highway", "motorway" },
    { "hwtag", "oneway" },
    { "psurface", "paved_good" },
    { "junction", "circular" },
  };

  AddTypes(params, arr2);
  params.FinishAddingTypes();
  fb1.SetParams(params);

  auto const diff = 0.33333333334567;
  for (size_t i = 0; i < 100; ++i)
    fb1.AddPoint(m2::PointD(i + diff, i + 1 + diff));

  fb1.SetLinear();

  TEST(fb1.RemoveInvalidTypes(), ());
  TEST(fb1.IsValid(), (fb1));

  FeatureBuilder::Buffer buffer;
  TEST(fb1.PreSerializeAndRemoveUselessNamesForIntermediate(), ());
  fb1.SerializeAccuratelyForIntermediate(buffer);

  FeatureBuilder fb2;
  fb2.DeserializeAccuratelyFromIntermediate(buffer);

  TEST(fb2.IsValid(), (fb2));
  TEST(fb1.IsExactEq(fb2), ());

  TEST_EQUAL(fb2.GetTypesCount(), 5, ());
  ftypes::IsRoundAboutChecker::Instance()(fb2.GetTypes());
}

UNIT_CLASS_TEST(TestWithClassificator, FBuilder_RemoveUselessAltName)
{
  auto const kDefault = StringUtf8Multilang::kDefaultCode;
  auto const kAltName = StringUtf8Multilang::GetLangIndex("alt_name");

  {
    FeatureBuilderParams params;

    char const * arr[][1] = {{"shop"}};
    AddTypes(params, arr);
    params.FinishAddingTypes();

    // We should remove alt_name which is almost equal to name.
    params.AddName("default", "Перекрёсток");
    params.AddName("alt_name", "Перекресток");

    FeatureBuilder fb;
    fb.SetParams(params);

    fb.SetCenter(m2::PointD(0.0, 0.0));

    TEST(!fb.GetName(kDefault).empty(), ());
    TEST(!fb.GetName(kAltName).empty(), ());

    fb.RemoveUselessNames();

    TEST(!fb.GetName(kDefault).empty(), ());
    TEST(fb.GetName(kAltName).empty(), ());

    TEST(fb.IsValid(), (fb));
  }
  {
    FeatureBuilderParams params;

    char const * arr[][1] = {{"shop"}};
    AddTypes(params, arr);
    params.FinishAddingTypes();

    // We should not remove alt_name which differs from name.
    params.AddName("default", "Государственный Универсальный Магазин");
    params.AddName("alt_name", "ГУМ");

    FeatureBuilder fb;
    fb.SetParams(params);

    fb.SetCenter(m2::PointD(0.0, 0.0));

    TEST(!fb.GetName(kDefault).empty(), ());
    TEST(!fb.GetName(StringUtf8Multilang::GetLangIndex("alt_name")).empty(), ());

    fb.RemoveUselessNames();

    TEST(!fb.GetName(kDefault).empty(), ());
    TEST(!fb.GetName(kAltName).empty(), ());

    TEST(fb.IsValid(), (fb));
  }
}
}  // namespace feature_builder_test
