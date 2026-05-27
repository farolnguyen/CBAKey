#include <cassert>

#include "farolkey/core/vi_syllable.h"

using farolkey::core::vi_syllable::findLastSyllable;
using farolkey::core::vi_syllable::findStableComposeSplit;
using farolkey::core::vi_syllable::applyTelexTransform;
using farolkey::core::vi_syllable::applyVniTransform;
using farolkey::core::vi_syllable::normalizeTelexBuffer;
using farolkey::core::vi_syllable::normalizeVietnameseNfc;
using farolkey::core::vi_syllable::removeTelexDiacritics;
using farolkey::core::vi_syllable::removeVniDiacritics;
using farolkey::core::vi_syllable::selectToneVowelIndex;
using farolkey::core::vi_syllable::normalizeSyllableTonePlacement;
using farolkey::core::vi_syllable::segmentWholeBufferPreferMaxSyllables;

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

    const auto splitXinChao = findStableComposeSplit(U"xinchao");
    assert(splitXinChao.has_value());
    assert(splitXinChao->committed_prefix_end == 3);
    assert(splitXinChao->active_suffix.begin == 3);
    assert(splitXinChao->active_suffix.onset_end == 5);

    const auto splitChaoBan = findStableComposeSplit(U"chaoban");
    assert(splitChaoBan.has_value());
    assert(splitChaoBan->committed_prefix_end == 4);

    assert(!findStableComposeSplit(U"xina").has_value());
    assert(!findStableComposeSplit(U"tieeng").has_value());

    assert(selectToneVowelIndex(U"chao") == 2);
    assert(selectToneVowelIndex(U"ho\u0103c") == 2);  // hoăc: tone on ă (oă → "oa" pattern)
    assert(selectToneVowelIndex(U"hieu") == 2);
    assert(selectToneVowelIndex(U"giư") == 2);
    assert(selectToneVowelIndex(U"thuơ") == 3);
    assert(selectToneVowelIndex(U"thuo") == 3);
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
    // "ua" with coda: tone goes on second vowel (a/â), not u.
    // e.g. "xuân" / "tuần" / "chuẩn" all have tone on â, not u.
    assert(selectToneVowelIndex(U"tuan") == 2);    // t-uan: tone on a(2)
    assert(selectToneVowelIndex(U"xu\u00e2n") == 2);  // x-u-â-n: tone on â(2)
    assert(selectToneVowelIndex(U"chu\u00e2n") == 3);  // ch-u-â-n: tone on â(3)
    assert(selectToneVowelIndex(U"thuy") == 3);
    assert(selectToneVowelIndex(U"khuây") == 3);
    assert(selectToneVowelIndex(U"rươu") == 2);
    assert(selectToneVowelIndex(U"tôi") == 1);
    assert(selectToneVowelIndex(U"bơi") == 1);
    assert(selectToneVowelIndex(U"tui") == 1);
    assert(selectToneVowelIndex(U"gưi") == 1);
    assert(selectToneVowelIndex(U"xoa") == 1);
    assert(selectToneVowelIndex(U"khoe") == 3);  // kh+oe: tone on 'e' (index 3)
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
    assert(selectToneVowelIndex(U"xoe") == 2);  // x+oe: tone on 'e' (index 2)
    assert(selectToneVowelIndex(U"huê") == 2);
    assert(selectToneVowelIndex(U"buôn") == 2);
    assert(selectToneVowelIndex(U"bưa") == 1);
    assert(selectToneVowelIndex(U"bươu") == 2);
    assert(selectToneVowelIndex(U"huy") == 2);
    assert(selectToneVowelIndex(U"lâu") == 1);
    assert(selectToneVowelIndex(U"bây") == 1);
    assert(selectToneVowelIndex(U"đều") == 1);
    assert(selectToneVowelIndex(U"nói") == 1);
    assert(selectToneVowelIndex(U"cối") == 1);
    assert(selectToneVowelIndex(U"mời") == 1);
    assert(selectToneVowelIndex(U"bụi") == 1);
    assert(selectToneVowelIndex(U"ngửi") == 2);
    assert(selectToneVowelIndex(U"múa") == 1);
    assert(selectToneVowelIndex(U"hoài") == 2);
    assert(selectToneVowelIndex(U"ngoáy") == 3);
    assert(selectToneVowelIndex(U"hữu") == 1);
    assert(selectToneVowelIndex(U"thiếu") == 3);
    assert(selectToneVowelIndex(U"yểu") == 1);
    assert(selectToneVowelIndex(U"khuya") == 3);
    assert(selectToneVowelIndex(U"khuay") == 3);
    assert(selectToneVowelIndex(U"huây") == 2);
    assert(selectToneVowelIndex(U"huơ") == 2);
    assert(selectToneVowelIndex(U"tuoi") == 2);
    assert(selectToneVowelIndex(U"cuơi") == 2);
    assert(selectToneVowelIndex(U"hưo") == 2);
    assert(selectToneVowelIndex(U"ruou") == 2);
    assert(selectToneVowelIndex(U"ruơu") == 2);
    assert(selectToneVowelIndex(U"rưou") == 2);
    assert(selectToneVowelIndex(U"hue") == 2);
    assert(selectToneVowelIndex(U"huu") == 1);

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

    std::u32string deuLateD = U"deu";
    assert(applyVniTransform(deuLateD, '9'));
    assert(deuLateD == U"đeu");

    std::u32string deuLateDToned = U"dèu";
    assert(applyVniTransform(deuLateDToned, '9'));
    assert(deuLateDToned == U"đèu");

    std::u32string thuocVni = U"thuo";
    assert(applyVniTransform(thuocVni, '6'));
    assert(thuocVni == U"thuô");

    std::u32string cuoiTone = U"cuói";
    assert(applyVniTransform(cuoiTone, '7'));
    assert(cuoiTone == U"cuới");

    std::u32string huongTone = U"hưó";
    assert(applyVniTransform(huongTone, '7'));
    assert(huongTone == U"hướ");

    std::u32string ruou = U"ruou";
    assert(applyVniTransform(ruou, '7'));
    assert(ruou == U"ruơu");
    assert(applyVniTransform(ruou, '7'));
    assert(ruou == U"rươu");

    std::u32string ruouToned = U"ruợu";
    assert(applyVniTransform(ruouToned, '7'));
    assert(ruouToned == U"rượu");

    std::u32string ruouLeading = U"rưou";
    assert(applyVniTransform(ruouLeading, '7'));
    assert(ruouLeading == U"rươu");

    std::u32string huu = U"huu";
    assert(applyVniTransform(huu, '7'));
    assert(huu == U"hưu");

    std::u32string huuToned = U"hũu";
    assert(applyVniTransform(huuToned, '7'));
    assert(huuToned == U"hữu");

    std::u32string thue = U"thue";
    assert(applyTelexTransform(thue, 'e'));
    assert(thue == U"thuê");

    std::u32string cuo = U"cuo";
    assert(applyTelexTransform(cuo, 'w'));
    assert(cuo == U"cuơ");

    std::u32string thuo = U"thuo";
    assert(applyTelexTransform(thuo, 'w'));
    assert(thuo == U"thuơ");

    // Bug fix: "oa"-glide nucleus — 'w' targets 'a'→'ă', not 'o'→'ơ'.
    // Typing "hoặc": hoa + w → hoă (not hơa), then j → hoặ, then c → hoặc.
    {
        std::u32string hoaw = U"hoa";
        assert(applyTelexTransform(hoaw, 'w'));
        assert(hoaw == U"hoă");  // hoă (U+0103), NOT hơa
    }

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

    std::u32string decomposedCircumflexAcute = U"a\u0302\u0301";
    assert(normalizeVietnameseNfc(decomposedCircumflexAcute));
    assert(decomposedCircumflexAcute == U"ấ");

    std::u32string decomposedAcuteCircumflex = U"a\u0301\u0302";
    assert(normalizeVietnameseNfc(decomposedAcuteCircumflex));
    assert(decomposedAcuteCircumflex == U"ấ");

    std::u32string decomposedWord = U"thu" U"e\u0302\u0301";
    assert(normalizeVietnameseNfc(decomposedWord));
    assert(decomposedWord == U"thuế");

    std::u32string decomposedHornDot = U"u\u031b\u0323";
    assert(normalizeVietnameseNfc(decomposedHornDot));
    assert(decomposedHornDot == U"ự");

    std::u32string decomposedBreveDot = U"a\u0306\u0323";
    assert(normalizeVietnameseNfc(decomposedBreveDot));
    assert(decomposedBreveDot == U"ặ");

    std::u32string decomposedDotBreve = U"a\u0323\u0306";
    assert(normalizeVietnameseNfc(decomposedDotBreve));
    assert(decomposedDotBreve == U"ặ");

    std::u32string decomposedTildeCircumflex = U"o\u0303\u0302";
    assert(normalizeVietnameseNfc(decomposedTildeCircumflex));
    assert(decomposedTildeCircumflex == U"ỗ");

    std::u32string decomposedAcuteHorn = U"u\u0301\u031b";
    assert(normalizeVietnameseNfc(decomposedAcuteHorn));
    assert(decomposedAcuteHorn == U"ứ");

    std::u32string decomposedNguyen = U"nguye\u0302\u0303n";
    assert(normalizeVietnameseNfc(decomposedNguyen));
    assert(decomposedNguyen == U"nguyễn");

    std::u32string alreadyNfc = U"nguyễn";
    assert(!normalizeVietnameseNfc(alreadyNfc));
    assert(alreadyNfc == U"nguyễn");

    std::u32string toneThenAa = U"á";
    assert(applyTelexTransform(toneThenAa, 'a'));
    assert(toneThenAa == U"ấ");

    std::u32string toneThenEe = U"é";
    assert(applyTelexTransform(toneThenEe, 'e'));
    assert(toneThenEe == U"ế");

    std::u32string toneThenOo = U"ó";
    assert(applyTelexTransform(toneThenOo, 'o'));
    assert(toneThenOo == U"ố");

    std::u32string toneThenOw = U"ó";
    assert(applyTelexTransform(toneThenOw, 'w'));
    assert(toneThenOw == U"ớ");

    std::u32string toneThenUw = U"ú";
    assert(applyTelexTransform(toneThenUw, 'w'));
    assert(toneThenUw == U"ứ");

    std::u32string erased = U"điều";
    assert(removeTelexDiacritics(erased));
    assert(erased == U"dieu");

    std::u32string erasedUow = U"uơ";
    assert(removeTelexDiacritics(erasedUow));
    assert(erasedUow == U"uo");

    std::u32string plain = U"abc";
    assert(!removeTelexDiacritics(plain));
    assert(plain == U"abc");

    std::u32string erasedVni = U"điều";
    assert(removeVniDiacritics(erasedVni));
    assert(erasedVni == U"dieu");

    std::u32string erasedDecomposedVni = U"thu" U"e\u0302\u0301";
    assert(removeVniDiacritics(erasedDecomposedVni));
    assert(erasedDecomposedVni == U"thue");

    std::u32string erasedDecomposedTelex = U"a\u0306\u0323";
    assert(removeTelexDiacritics(erasedDecomposedTelex));
    assert(erasedDecomposedTelex == U"a");

    std::u32string erasedDecomposedWord = U"nguye\u0302\u0303n";
    assert(removeVniDiacritics(erasedDecomposedWord));
    assert(erasedDecomposedWord == U"nguyen");

    std::u32string plainVni = U"abc";
    assert(!removeVniDiacritics(plainVni));
    assert(plainVni == U"abc");

    // Typo "chủân" / "chủần": strip only the last syllable (ân / ần -> an).
    const auto segChuanTypo = segmentWholeBufferPreferMaxSyllables(U"ch\u1EE7\u00E2n");
    assert(segChuanTypo.has_value() && segChuanTypo->size() == 2);
    std::u32string chuanTypo = U"ch\u1EE7\u00E2n";
    assert(removeTelexDiacritics(chuanTypo));
    assert(chuanTypo == U"ch\u1EE7an");
    chuanTypo = U"Ch\u1EE7\u00E2n";
    assert(removeVniDiacritics(chuanTypo));
    assert(chuanTypo == U"Ch\u1EE7an");

    const auto segChuanSac = segmentWholeBufferPreferMaxSyllables(U"ch\u1EE7\u1EA7n");
    assert(segChuanSac.has_value() && segChuanSac->size() == 2);
    chuanTypo = U"ch\u1EE7\u1EA7n";
    assert(removeTelexDiacritics(chuanTypo));
    assert(chuanTypo == U"ch\u1EE7an");

    // NFD (decomposed ủ + decomposed â) must NFC-merge then strip like precomposed "chủân".
    std::u32string nfdChuan = U"ch\u0075\u0309\u0061\u0302n";
    assert(normalizeVietnameseNfc(nfdChuan));
    assert(nfdChuan == U"ch\u1EE7\u00E2n");
    assert(removeTelexDiacritics(nfdChuan));
    assert(nfdChuan == U"ch\u1EE7an");

    // Uppercase G + "íup": same syllable as "giúp"; strip must not peel tone onto bare "i".
    std::u32string giupUpper = U"G\u00edup";
    assert(removeTelexDiacritics(giupUpper));
    assert(giupUpper == U"Giup");

    // --- Uppercase Đ: VNI D+9 → Đ ---
    {
        std::u32string buf = U"D";  // user typed Shift+D
        assert(applyVniTransform(buf, '9'));
        assert(buf == U"Đ");
    }
    {
        // Uppercase onset in full syllable: "Dau" + 9 → "Đau"
        std::u32string buf = U"Dau";
        assert(applyVniTransform(buf, '9'));
        assert(buf == U"Đau");
    }
    {
        // Lowercase still works
        std::u32string buf = U"d";
        assert(applyVniTransform(buf, '9'));
        assert(buf == U"đ");
    }

    // --- Uppercase Đ: Telex D+d → Đ (second key lowercase) ---
    {
        std::u32string buf = U"D";  // user typed Shift+D
        assert(applyTelexTransform(buf, 'd'));  // second key d
        assert(buf == U"Đ");
    }
    {
        // D+D (both uppercase, key lowercased by engine to 'd')
        std::u32string buf = U"D";
        assert(applyTelexTransform(buf, 'd'));
        assert(buf == U"Đ");
    }
    {
        // Lowercase still works
        std::u32string buf = U"d";
        assert(applyTelexTransform(buf, 'd'));
        assert(buf == U"đ");
    }

    // M17.2: normalizeSyllableTonePlacement — tone movement + glide diacritic strip.
    // Case 1: tone on 'o' glide → move to 'ă' (e.g. Telex h→o→j→a→w→c, VNI h→o→5→a→8→c)
    // "họăc" (h + ọ + ă + c) → "hoặc"
    {
        std::u32string s = U"họăc";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hoặc");  // hoặc
    }
    // Case 2: horn diacritic on glide, tone already on correct 'ặ' → strip ơ→o
    // "hơặc" → "hoặc"
    {
        std::u32string s = U"hơặc";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hoặc");
    }
    // Case 3: horn diacritic on glide, no tone → strip ơ→o
    // "hơăc" → "hoăc"
    {
        std::u32string s = U"hơăc";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hoăc");
    }
    // Case 4: tone AND horn on glide (h→o→w→j→a→c) — "hợac" → "hoạc"
    {
        std::u32string s = U"hợac";  // hợac (ợ = ơ+nặng)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hoạc");  // hoạc (ạ = a+nặng)
    }
    // Case 5: already correct → no change
    {
        std::u32string s = U"hoặc";  // hoặc
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hoặc");
    }
    // Case 6: open syllable "họa" — tone on 'ọ' is correct (open "oa" → offset=0) → no change
    {
        std::u32string s = U"họa";  // họa
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"họa");
    }

    // M17 yGlide: tone misplaced on 'y' glide → move to 'ê'.
    // Case 7: "quỳên" → "quyền" (huyền on ỳ → ề)
    {
        std::u32string s = U"quỳên";  // quỳên (ỳ=U+1EF3, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"quyền");  // quyền (ề=U+1EC1)
    }
    // Case 8: "quyền" already correct → no change
    {
        std::u32string s = U"quyền";  // quyền
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"quyền");
    }

    // M17 uGlide: tone misplaced on 'ư' → move to 'ơ'-family vowel.
    // Case 9: "cứơi" → "cưới" (sắc on ứ → ớ, nucleus "ươi")
    {
        std::u32string s = U"cứơi";  // cứơi (ứ=U+1EE9, ơ=U+01A1)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"cưới");  // cưới (ư=U+01B0, ớ=U+1EDB)
    }
    // Case 10: "cưới" already correct → no change
    {
        std::u32string s = U"cưới";  // cưới
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"cưới");
    }
    // Case 11: "hứơng" → "hướng" (sắc on ứ → ớ, nucleus "ươ" + coda "ng")
    {
        std::u32string s = U"hứơng";  // hứơng
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"hướng");  // hướng (ư=U+01B0, ớ=U+1EDB)
    }
    // Case 12: "thuỷ" — nucleus "uy" (b1='y') → NOT uGlide → no change
    {
        std::u32string s = U"thuỷ";  // thuỷ (ỷ=U+1EF7 = y+hỏi)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"thuỷ");
    }

    // M17.6 uyGlide: "uy*" nucleus (nucleusLen≥3) — tone misplaced on u or y → move to ê.
    // Case 13: "ngùyên" → "nguyền" (huyền on 'u' at nucleus[0], moves to 'ê' at [2])
    {
        std::u32string s = U"ngùyên";  // ngùyên (ù=U+00F9, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"nguyền");  // nguyền (ề=U+1EC1)
    }
    // Case 14: "nguỳên" → "nguyền" (huyền on 'y' at nucleus[1], moves to 'ê' at [2])
    {
        std::u32string s = U"nguỳên";  // nguỳên (ỳ=U+1EF3, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"nguyền");  // nguyền
    }
    // Case 15: "nguyền" already correct → no change
    {
        std::u32string s = U"nguyền";  // nguyền
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"nguyền");
    }
    // Case 16: "thủy" — nucleusLen=2 → uyGlide NOT triggered → no change
    {
        std::u32string s = U"thủy";  // thủy (ủ=U+1EE7)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"thủy");
    }

    // M17.7 ieGlide: tone misplaced on 'i' in "ie*"/"iê*" nucleus → move to 'ê'.
    // Case 17: "tíêng" → "tiếng" (sắc on 'i', 2-char nucleus + coda "ng")
    {
        std::u32string s = U"tíêng";  // tíêng (í=U+00ED, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tiếng");  // tiếng (ế=U+1EBF)
    }
    // Case 18: "tiếng" already correct → no change
    {
        std::u32string s = U"tiếng";  // tiếng
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tiếng");
    }
    // Case 19: "mìên" → "miền" (huyền on 'i', 2-char nucleus + coda "n")
    {
        std::u32string s = U"mìên";  // mìên (ì=U+00EC, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"miền");  // miền (ề=U+1EC1)
    }
    // Case 20: "miền" already correct → no change
    {
        std::u32string s = U"miền";  // miền
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"miền");
    }

    // M17.8 ueGlide: tone misplaced on 'u' in "ue"/"uê" nucleus → move to 'ê'.
    // Case 21: "thúê" → "thuế" (sắc on 'u', open nucleus "uê")
    {
        std::u32string s = U"thúê";  // thúê (ú=U+00FA, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"thuế");  // thuế (ế=U+1EBF)
    }
    // Case 22: "thuế" already correct → no change
    {
        std::u32string s = U"thuế";  // thuế
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"thuế");
    }
    // Case 23: "tụê" → "tuệ" (nặng on 'u', open nucleus "uê")
    {
        std::u32string s = U"tụê";  // tụê (ụ=U+1EE5, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tuệ");  // tuệ (ệ=U+1EC7)
    }
    // Case 24: "tuệ" already correct → no change
    {
        std::u32string s = U"tuệ";  // tuệ
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tuệ");
    }

    // M17.10 oeGlide: 'o' is glide in "oe"; tone always goes to 'e' (open and closed).
    // Case 31: "khóe" → "khoé" (sắc on 'o', open syllable)
    {
        std::u32string s = U"khóe";  // khóe (ó=U+00F3)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"khoé");  // khoé (é=U+00E9)
    }
    // Case 32: "khoé" already correct → no change
    {
        std::u32string s = U"khoé";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"khoé");
    }
    // Case 33: "xòe" → "xoè" (huyền on 'o', open syllable)
    {
        std::u32string s = U"xòe";  // xòe (ò=U+00F2)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"xoè");  // xoè (è=U+00E8)
    }
    // Case 34: "xoè" already correct → no change
    {
        std::u32string s = U"xoè";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"xoè");
    }

    // M17.11 uaGlide: 'u'/'ư' glide before 'a'; tone follows selectToneOffset rule
    // (open → 'u', closed → 'a'). correctToneBearingIndex handles open/closed automatically.
    // Case 35: "túan" → "tuán" (sắc on 'u', closed with coda 'n', should be on 'a')
    {
        std::u32string s = U"túan";  // túan (ú=U+00FA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tuán");  // tuán (á=U+00E1)
    }
    // Case 36: "tuán" already correct → no change
    {
        std::u32string s = U"tuán";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"tuán");
    }
    // Case 37: "muá" → "múa" (sắc on 'a', open syllable "ua" → tone should be on 'u')
    {
        std::u32string s = U"muá";  // muá (á=U+00E1)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"múa");  // múa (ú=U+00FA)
    }
    // Case 38: "múa" already correct → no change
    {
        std::u32string s = U"múa";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"múa");
    }

    // M17.9 iMedial/uMedial: tone on semivowel medial → move to nucleus.
    // Case 25: "gíup" → "giúp" ('i' medial after 'g', nucleus 'u', coda 'p')
    {
        std::u32string s = U"gíup";  // gíup (í=U+00ED)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"giúp");  // giúp (ú=U+00FA)
    }
    // Case 26: "giúp" already correct → no change
    {
        std::u32string s = U"giúp";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"giúp");
    }
    // Case 27: "qúa" → "quá" ('u' medial after 'q', nucleus 'a', open syllable)
    {
        std::u32string s = U"qúa";  // qúa (ú=U+00FA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"quá");  // quá (á=U+00E1)
    }
    // Case 28: "quá" already correct → no change
    {
        std::u32string s = U"quá";
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(!normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"quá");
    }
    // Case 29: "qùyên" → "quyền" ('u' medial after 'q', nucleus 'yê' + coda 'n')
    {
        std::u32string s = U"qùyên";  // qùyên (ù=U+00F9, ê=U+00EA)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"quyền");  // quyền (ề=U+1EC1)
    }
    // Case 30: "gíam" → "giám" ('i' medial after 'g', nucleus 'a', coda 'm')
    {
        std::u32string s = U"gíam";  // gíam (í=U+00ED)
        const auto sp = findLastSyllable(s);
        assert(sp.has_value());
        assert(normalizeSyllableTonePlacement(s, *sp));
        assert(s == U"giám");  // giám (á=U+00E1)
    }

    return 0;
}
