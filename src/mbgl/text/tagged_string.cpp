#include <mbgl/text/tagged_string.hpp>
#include <mbgl/math/minmax.hpp>
#include <mbgl/util/i18n.hpp>

namespace {
char16_t PUAbegin = u'\uE000';
char16_t PUAend = u'\uF8FF';
} // namespace

namespace mbgl {

void TaggedString::addTextSection(const std::u16string& sectionText,
                                  double scale,
                                  FontStack fontStack,
                                  optional<Color> textColor) {
    styledText.first += sectionText;
    sections.emplace_back(scale, fontStack, std::move(textColor));
    styledText.second.resize(styledText.first.size(), sections.size() - 1);
    supportsVerticalWritingMode = nullopt;
}

void TaggedString::addImageSection(const std::string& imageID) {
    const auto& nextImageSectionCharCode = getNextImageSectionCharCode();
    if (!nextImageSectionCharCode) {
        return;
    }

    styledText.first += *nextImageSectionCharCode;
    sections.emplace_back(imageID);
    styledText.second.resize(styledText.first.size(), sections.size() - 1);
}

optional<char16_t> TaggedString::getNextImageSectionCharCode() {
    if (!imageSectionID) {
        imageSectionID = PUAbegin;
        return imageSectionID;
    }

    if (*imageSectionID >= PUAend) {
        assert(false);
        return nullopt;
    }

    (*imageSectionID)++;
    return imageSectionID;
}

void TaggedString::trim() {
    std::size_t beginningWhitespace = styledText.first.find_first_not_of(u" \t\n\v\f\r");
    if (beginningWhitespace == std::u16string::npos) {
        // Entirely whitespace
        styledText.first.clear();
        styledText.second.clear();
    } else {
        std::size_t trailingWhitespace = styledText.first.find_last_not_of(u" \t\n\v\f\r") + 1;

        styledText.first = styledText.first.substr(beginningWhitespace, trailingWhitespace - beginningWhitespace);
        styledText.second = std::vector<uint8_t>(styledText.second.begin() + beginningWhitespace, styledText.second.begin() + trailingWhitespace);
    }
}

double TaggedString::getMaxScale() const {
    double maxScale = 0.0;
    for (std::size_t i = 0; i < styledText.first.length(); i++) {
        maxScale = util::max(maxScale, getSection(i).scale);
    }
    return maxScale;
}
    
void TaggedString::verticalizePunctuation() {
    // Relies on verticalization changing characters in place so that style indices don't need updating
    styledText.first = util::i18n::verticalizePunctuation(styledText.first);
}

bool TaggedString::allowsVerticalWritingMode() {
    if (!supportsVerticalWritingMode) {
        supportsVerticalWritingMode = util::i18n::allowsVerticalWritingMode(rawText());
    }
    return *supportsVerticalWritingMode;
}

} // namespace mbgl
