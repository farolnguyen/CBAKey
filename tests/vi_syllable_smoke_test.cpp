#include <cassert>

#include "cbakey/core/vi_syllable.h"

using cbakey::core::vi_syllable::findLastSyllable;
using cbakey::core::vi_syllable::applyTelexTransform;
using cbakey::core::vi_syllable::applyVniTransform;
using cbakey::core::vi_syllable::normalizeTelexBuffer;
using cbakey::core::vi_syllable::selectToneVowelIndex;

int main() {
    const auto qua = findLastSyllable(U"qua");
    assert(qua.has_value());
    assert(qua->begin == 0);
    assert(qua->onset_end == 1);
    assert(qua->medial_end == 2);
    assert(qua->nucleus_end == 3);
    assert(qua->end == 3);
    assert(selectToneVowelIndex(U"qua") == 2);

    const auto gia = findLastSyllable(U"gia");
    assert(gia.has_value());
    assert(gia->onset_end == 1);
    assert(gia->medial_end == 2);
    assert(gia->nucleus_end == 3);
    assert(selectToneVowelIndex(U"gia") == 2);

    assert(selectToneVowelIndex(U"chao") == 2);
    assert(selectToneVowelIndex(U"hieu") == 2);
    assert(selectToneVowelIndex(U"giư") == 2);
    assert(selectToneVowelIndex(U"thuơ") == 3);
    assert(selectToneVowelIndex(U"thuyê") == 4);
    assert(selectToneVowelIndex(U"nguyê") == 4);
    assert(selectToneVowelIndex(U"vietnam") == 5);
    assert(selectToneVowelIndex(U"ngoai") == 3);
    assert(selectToneVowelIndex(U"xoay") == 2);
    assert(selectToneVowelIndex(U"thuê") == 3);
    assert(selectToneVowelIndex(U"tuôi") == 2);
    assert(selectToneVowelIndex(U"cươi") == 2);
    assert(selectToneVowelIndex(U"gâu") == 1);
    assert(selectToneVowelIndex(U"cây") == 1);
    assert(selectToneVowelIndex(U"nêu") == 1);
    assert(selectToneVowelIndex(U"cưu") == 1);
    assert(selectToneVowelIndex(U"hiêu") == 2);
    assert(selectToneVowelIndex(U"yêu") == 1);
    assert(selectToneVowelIndex(U"muôi") == 2);
    assert(selectToneVowelIndex(U"tươi") == 2);
    assert(selectToneVowelIndex(U"mia") == 1);
    assert(selectToneVowelIndex(U"cua") == 1);
    assert(selectToneVowelIndex(U"hưa") == 1);
    assert(selectToneVowelIndex(U"thuy") == 3);
    assert(selectToneVowelIndex(U"khuây") == 3);
    assert(selectToneVowelIndex(U"rươu") == 2);
    assert(selectToneVowelIndex(U"tôi") == 1);
    assert(selectToneVowelIndex(U"bơi") == 1);
    assert(selectToneVowelIndex(U"tui") == 1);
    assert(selectToneVowelIndex(U"gưi") == 1);
    assert(selectToneVowelIndex(U"xoa") == 1);
    assert(selectToneVowelIndex(U"khoe") == 2);
    assert(selectToneVowelIndex(U"toan") == 2);
    assert(selectToneVowelIndex(U"gai") == 1);
    assert(selectToneVowelIndex(U"may") == 1);
    assert(selectToneVowelIndex(U"sau") == 1);
    assert(selectToneVowelIndex(U"kheo") == 2);
    assert(selectToneVowelIndex(U"bao") == 1);
    assert(selectToneVowelIndex(U"cai") == 1);
    assert(selectToneVowelIndex(U"hay") == 1);
    assert(selectToneVowelIndex(U"chau") == 2);
    assert(selectToneVowelIndex(U"beo") == 1);
    assert(selectToneVowelIndex(U"kia") == 1);
    assert(selectToneVowelIndex(U"xoe") == 1);
    assert(selectToneVowelIndex(U"huê") == 2);
    assert(selectToneVowelIndex(U"buôn") == 2);
    assert(selectToneVowelIndex(U"bưa") == 1);
    assert(selectToneVowelIndex(U"bươu") == 2);

    std::u32string huong = U"huo";
    assert(applyTelexTransform(huong, 'w'));
    assert(huong == U"huơ");
    huong.push_back(U'n');
    assert(normalizeTelexBuffer(huong));
    assert(huong == U"hươn");

    std::u32string tieng = U"tie";
    assert(applyTelexTransform(tieng, 'e'));
    assert(tieng == U"tiê");

    std::u32string huongVni = U"hưo";
    assert(applyVniTransform(huongVni, '7'));
    assert(huongVni == U"hươ");

    std::u32string thuocVni = U"thuo";
    assert(applyVniTransform(thuocVni, '6'));
    assert(thuocVni == U"thuô");

    std::u32string thue = U"thue";
    assert(applyTelexTransform(thue, 'e'));
    assert(thue == U"thuê");

    std::u32string cuo = U"cuo";
    assert(applyTelexTransform(cuo, 'w'));
    assert(cuo == U"cuơ");

    std::u32string thuo = U"thuo";
    assert(applyTelexTransform(thuo, 'w'));
    assert(thuo == U"thuơ");

    std::u32string gau = U"ga";
    assert(applyTelexTransform(gau, 'a'));
    assert(gau == U"gâ");

    std::u32string neu = U"ne";
    assert(applyTelexTransform(neu, 'e'));
    assert(neu == U"nê");

    std::u32string hieu = U"hie";
    assert(applyTelexTransform(hieu, 'e'));
    assert(hieu == U"hiê");

    std::u32string yeu = U"ye";
    assert(applyTelexTransform(yeu, 'e'));
    assert(yeu == U"yê");

    return 0;
}
