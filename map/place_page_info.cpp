#include "place_page_info.hpp"

#include "indexer/feature_utils.hpp"
#include "indexer/osm_editor.hpp"

#include "platform/measurement_utils.hpp"
#include "platform/preferred_languages.hpp"

namespace place_page
{
char const * const Info::kSubtitleSeparator = " • ";
char const * const Info::kStarSymbol = "★";
char const * const Info::kMountainSymbol = "▲";
char const * const Info::kEmptyRatingSymbol = "-";
char const * const Info::kPricingSymbol = "$";

bool Info::IsFeature() const { return m_featureID.IsValid(); }
bool Info::IsBookmark() const { return m_bac.IsValid(); }
bool Info::IsMyPosition() const { return m_isMyPosition; }
bool Info::IsSponsored() const { return m_sponsoredType != SponsoredType::None; }
bool Info::ShouldShowAddPlace() const
{
  auto const isPointOrBuilding = IsPointType() || IsBuilding();
  return m_canEditOrAdd && !(IsFeature() && isPointOrBuilding);
}

bool Info::ShouldShowAddBusiness() const { return m_canEditOrAdd && IsBuilding(); }

bool Info::ShouldShowEditPlace() const
{
  return m_canEditOrAdd &&
         // TODO(mgsergio): Does IsFeature() imply !IsMyPosition()?
         !IsMyPosition() && IsFeature();
}

bool Info::HasApiUrl() const { return !m_apiUrl.empty(); }
bool Info::HasWifi() const { return GetInternet() == osm::Internet::Wlan; }

bool Info::HasBanner() const
{
  return !m_banner.IsEmpty() && m_banner.IsActive();
}

string Info::FormatNewBookmarkName() const
{
  string const title = GetTitle();
  if (title.empty())
    return GetLocalizedType();
  return title;
}

string Info::GetTitle() const
{
  if (!m_customName.empty())
    return m_customName;

  string name;
  auto const deviceLang = StringUtf8Multilang::GetLangIndex(languages::GetCurrentNorm());

  auto const mwmInfo = GetID().m_mwmId.GetInfo();

  if (mwmInfo)
    feature::GetReadableName(mwmInfo->GetRegionData(), m_name, deviceLang, name);

  return name;
}

string Info::GetSubtitle() const
{
  if (!IsFeature())
  {
    if (IsBookmark())
      return m_bookmarkCategoryName;
    return {};
  }

  vector<string> values;

  // Bookmark category.
  if (IsBookmark())
    values.push_back(m_bookmarkCategoryName);

  // Type.
  values.push_back(GetLocalizedType());

  // Flats.
  string const flats = GetFlats();
  if (!flats.empty())
    values.push_back(flats);

  // Cuisines.
  for (string const & cuisine : GetLocalizedCuisines())
    values.push_back(cuisine);

  // Stars.
  string const stars = FormatStars();
  if (!stars.empty())
    values.push_back(stars);

  // Operator.
  string const op = GetOperator();
  if (!op.empty())
    values.push_back(op);

  // Elevation.
  string const eleStr = GetElevationFormatted();
  if (!eleStr.empty())
    values.push_back(kMountainSymbol + eleStr);
  if (HasWifi())
    values.push_back(m_localizedWifiString);

  return strings::JoinStrings(values, kSubtitleSeparator);
}

string Info::FormatStars() const
{
  string stars;
  for (int i = 0; i < GetStars(); ++i)
    stars.append(kStarSymbol);
  return stars;
}

string Info::GetFormattedCoordinate(bool isDMS) const
{
  auto const & ll = GetLatLon();
  return isDMS ? measurement_utils::FormatLatLon(ll.lat, ll.lon) : measurement_utils::FormatLatLonAsDMS(ll.lat, ll.lon, 2);
}

string Info::GetCustomName() const { return m_customName; }
BookmarkAndCategory Info::GetBookmarkAndCategory() const { return m_bac; }
string Info::GetBookmarkCategoryName() const { return m_bookmarkCategoryName; }
string const & Info::GetApiUrl() const { return m_apiUrl; }

string const & Info::GetSponsoredUrl() const { return m_sponsoredUrl; }
string const & Info::GetSponsoredDescriptionUrl() const {return m_sponsoredDescriptionUrl; }

string Info::GetRatingFormatted() const
{
  if (!IsSponsored())
    return string();

  auto const r = GetMetadata().Get(feature::Metadata::FMD_RATING);
  char const * rating = r.empty() ? kEmptyRatingSymbol : r.c_str();
  int const size = snprintf(nullptr, 0, m_localizedRatingString.c_str(), rating);
  if (size < 0)
  {
    LOG(LERROR, ("Incorrect size for string:", m_localizedRatingString, ", rating:", rating));
    return string();
  }

  vector<char> buf(size + 1);
  snprintf(buf.data(), buf.size(), m_localizedRatingString.c_str(), rating);
  return string(buf.begin(), buf.end());
}

string Info::GetApproximatePricing() const
{
  if (!IsSponsored())
    return string();

  int pricing;
  strings::to_int(GetMetadata().Get(feature::Metadata::FMD_PRICE_RATE), pricing);
  string result;
  for (auto i = 0; i < pricing; i++)
    result.append(kPricingSymbol);

  return result;
}

string Info::GetBannerTitleId() const
{
  CHECK(!m_banner.IsEmpty(), ());
  return m_banner.GetMessageBase() + "_title";
}

string Info::GetBannerMessageId() const
{
  CHECK(!m_banner.IsEmpty(), ());
  return m_banner.GetMessageBase() + "_message";
}

string Info::GetBannerIconId() const
{
  CHECK(!m_banner.IsEmpty(), ());
  return m_banner.GetIconName();
}

string Info::GetBannerUrl() const
{
  CHECK(!m_banner.IsEmpty(), ());
  return m_banner.GetFormattedUrl(m_metadata.Get(feature::Metadata::FMD_BANNER_URL));
}

void Info::SetMercator(m2::PointD const & mercator) { m_mercator = mercator; }
}  // namespace place_page
