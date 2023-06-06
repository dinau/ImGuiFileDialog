#include "Test_FilterManager.h"

#include <cassert>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

////////////////////////////////////////////////////////////////////////////
//// Filter use cases //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// ".*,.cpp,.h,.hpp" => simple filters
// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters
// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex
// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // CollectionFilters /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

IGFD::FilterManager Test_ParseFilters(const char* vFilters) {
    IGFD::FilterManager mgr;

    std::vector<IGFD::FilterManager::FilterInfos> prParsedFilters;
    IGFD::FilterManager::FilterInfos prSelectedFilter;
    std::string puDLGFilters;

#if 1
    mgr.ParseFilters(vFilters);
#else
    //////////////////////////////////////////////////////

    prParsedFilters.clear();

    if (vFilters)
        puDLGFilters = vFilters;  // file mode
    else
        puDLGFilters.clear();  // directory mode

    if (!puDLGFilters.empty()) {
        /* Rules
        0) a filter must have 2 chars mini and the first must be a .
        1) a regex must be in (( and ))
        2) a , will separate filters except if between a ( and )
        3) name{filter1, filter2} is a spetial form for collection filters
        3.1) the name can be composed of what you want except { and }
        3.2) the filter can be a regex
        4) the filters cannot integrate these chars '(' ')' '{' '}' ' ' except for a regex with respect to rule 1)
        5) the filters cannot integrate a ','
        */

        bool collection_started  = false;
        bool regex_started       = false;
        bool parenthesis_started = false;

        std::string word;
        std::string filter_name;

        char last_split_char = 0;
        for (char c : puDLGFilters) {
            if (c == '{') {
                if (regex_started) {
                    word += c;
                } else {
                    collection_started = true;
                    prParsedFilters.emplace_back();
                    prParsedFilters.back().filter = filter_name;
                    filter_name.clear();
                    word.clear();
                }
                last_split_char = c;
            } else if (c == '}') {
                if (regex_started) {
                    word += c;
                } else {
                    if (collection_started) {
                        if (word.size() > 1U && word[0] == '.') {
                            if (prParsedFilters.empty()) { prParsedFilters.emplace_back(); }
                            prParsedFilters.back().collectionfilters.emplace(word);
                        }
                        word.clear();
                        filter_name.clear();
                        collection_started = false;
                    }
                }
                last_split_char = c;
            } else if (c == '(') {
                word += c;
                if (last_split_char == '(') { regex_started = true; }
                parenthesis_started = true;
                if (!collection_started) { filter_name += c; }
                last_split_char = c;
            } else if (c == ')') {
                word += c;
                if (last_split_char == ')') {
                    if (regex_started) {
                        try {
                            auto rx = std::regex(word);  // cant thrwo an exception so if first, prevent emplace if failed
                            if (collection_started) {
                                prParsedFilters.back().collectionfilters.emplace(word);
                                prParsedFilters.back().collectionfilters_regex.emplace_back(rx);
                            } else {
                                prParsedFilters.emplace_back();
                                prParsedFilters.back().filter       = word;
                                prParsedFilters.back().filter_regex = rx;
                            }
                        } catch (std::exception&) {}
                        word.clear();
                        filter_name.clear();
                        regex_started = false;
                    } else {
                        if (!collection_started) {
                            if (!prParsedFilters.empty()) {
                                prParsedFilters.erase(prParsedFilters.begin() + prParsedFilters.size() - 1U);
                            } else {
                                prParsedFilters.clear();
                            }
                        }
                        word.clear();
                        filter_name.clear();
                    }
                }
                parenthesis_started = false;
                if (!collection_started) { filter_name += c; }
                last_split_char = c;
            } else if (c == '.') {
                word += c;
                if (!collection_started) { filter_name += c; }
                last_split_char = c;
            } else if (c == ',') {
                if (regex_started) {
                    regex_started = false;
                    word.clear();
                    filter_name.clear();
                } else {
                    if (collection_started) {
                        if (word.size() > 1U && word[0] == '.') {
                            prParsedFilters.back().collectionfilters.emplace(word);
                            word.clear();
                            filter_name.clear();
                        }
                    } else {
                        if (word.size() > 1U && word[0] == '.') {
                            prParsedFilters.emplace_back();
                            prParsedFilters.back().filter = word;
                            word.clear();
                            filter_name.clear();
                        }
                        if (parenthesis_started) { filter_name += c; }
                    }
                }
            } else {
                if (c != ' ') { word += c; }
                if (!collection_started) { filter_name += c; }
            }
        }

        if (collection_started) {
            if (!prParsedFilters.empty()) {
                prParsedFilters.erase(prParsedFilters.begin() + prParsedFilters.size() - 1U);
            } else {
                prParsedFilters.clear();
            }
        } else if (word.size() > 1U && word[0] == '.') {
            prParsedFilters.emplace_back();
            prParsedFilters.back().filter = word;
            word.clear();
        }

        bool currentFilterFound = false;

        for (const auto& it : prParsedFilters) {
            if (it.filter == prSelectedFilter.filter) {
                currentFilterFound = true;
                prSelectedFilter   = it;
            }
        }

        if (!currentFilterFound) {
            if (!prParsedFilters.empty()) prSelectedFilter = *prParsedFilters.begin();
        }
    }

    //////////////////////////////////////////////////////

    mgr.prParsedFilters = prParsedFilters;
#endif

    return mgr;
}

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Simple Filters ////////////////////////////////////////
//// ".*,.cpp,.h,.hpp" => simple filters ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#pragma region Filters Simple

bool Test_FilterManager_ParseFilters_Filters_Simple_0() {
    auto mgr = Test_ParseFilters(".*");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != ".*") return false;

    return true;
}

bool Test_FilterManager_ParseFilters_Filters_Simple_1() {
    auto mgr = Test_ParseFilters(".*,.cpp,.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != ".*") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// missing '.'
bool Test_FilterManager_ParseFilters_Filters_Simple_2() {
    auto mgr = Test_ParseFilters("hpp");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// empty filter after ','
bool Test_FilterManager_ParseFilters_Filters_Simple_3() {
    auto mgr = Test_ParseFilters(".hpp,");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != ".hpp") return false;

    return true;
}

// good filter
bool Test_FilterManager_ParseFilters_Filters_Simple_4() {
    auto mgr = Test_ParseFilters("..hpp");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;

    return true;
}

// good filter, but the second is empty
bool Test_FilterManager_ParseFilters_Filters_Simple_5() {
    auto mgr = Test_ParseFilters("..hpp,");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;

    return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_6() {
    auto mgr = Test_ParseFilters("..hpp,.hpp");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;

    return true;
}

// good filter, but the third not
bool Test_FilterManager_ParseFilters_Filters_Simple_7() {
    auto mgr = Test_ParseFilters("..hpp,.hpp,.");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;

    return true;
}

// good filters
bool Test_FilterManager_ParseFilters_Filters_Simple_8() {
    auto mgr = Test_ParseFilters("..hpp,.hpp,. .");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "..hpp") return false;
    if (mgr.prParsedFilters[1].filter != ".hpp") return false;
    if (mgr.prParsedFilters[2].filter != "..") return false;

    return true;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // CollectionFilters /////////////////////////////////////////////////////////////
//// "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md" => collection filters //////
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Collection

// must be good {filter0, filter1}
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_0() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// missing end }
// 2022/06/21 => give an infinite loop
bool Test_FilterManager_ParseFilters_Filters_Collection_1() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// missing start }
// 2022/06/21 => ok
bool Test_FilterManager_ParseFilters_Filters_Collection_2() {
    auto mgr = Test_ParseFilters("Source Files.cpp,.c}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_3() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx}");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_4() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c},Header Files{.hpp,.h,.hxx},");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_5() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx} , ");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}

// first { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_6() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files.hpp,.h,.hxx} , ");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_7() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx , .md");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_8() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_9() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md,Other Files{.txt,.doc}");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;
    if (mgr.prParsedFilters[3].filter != "Other Files") return false;
    if (mgr.prParsedFilters[3].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".txt") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".doc") == mgr.prParsedFilters[3].collectionfilters.end()) return false;

    return true;
}

// first { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_10() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files.txt,.doc}");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;

    return true;
}

// last { missing
bool Test_FilterManager_ParseFilters_Filters_Collection_11() {
    auto mgr = Test_ParseFilters("Source Files{.cpp,.c}, Header Files{.hpp,.h,.hxx},.md, Other Files{.txt,.doc");

    if (mgr.prParsedFilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].filter != "Source Files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header Files") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hpp") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".hxx") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != ".md") return false;

    return true;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////
//// ParseFilters // simple filters with regex /////////////////////////////
//// "([.][0-9]{3}),.cpp,.h,.hpp" => simple filters with regex /////////////
////////////////////////////////////////////////////////////////////////////

#pragma region Filters Simple Regex

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_0() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3}))");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_1() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3}");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_2() {
    auto mgr = Test_ParseFilters("[.][0-9]{3}))");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_3() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_4() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_5() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// last ) missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_6() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3},.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// first ( missing
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_7() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,[.][0-9]{3})),.h,.hpp");

    if (mgr.prParsedFilters.size() != 4U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != ".h") return false;
    if (mgr.prParsedFilters[3].filter != ".hpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_8() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})),.cpp,(([.][0-9]{3})) ,.h,.hpp");

    if (mgr.prParsedFilters.size() != 5U) return false;
    if (mgr.prParsedFilters[0].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[1].filter != ".cpp") return false;
    if (mgr.prParsedFilters[2].filter != "(([.][0-9]{3}))") return false;
    if (mgr.prParsedFilters[3].filter != ".h") return false;
    if (mgr.prParsedFilters[4].filter != ".hpp") return false;

    return true;
}

// must fail
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_9() {
    auto mgr = Test_ParseFilters("(([.][0-9]{3})");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

// must fail
bool Test_FilterManager_ParseFilters_Filters_Simple_Regex_10() {
    auto mgr = Test_ParseFilters("([.][0-9]{3}))");

    if (mgr.prParsedFilters.size() != 0U) return false;

    return true;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Collection filters with regex //////////////////////////////////////////
//// "frames files{([.][0-9]{3}),.frames}" => collection filters with regex /////////////
/////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Collection with Regex

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_0() {
    auto mgr = Test_ParseFilters("frames files{(([.][0-9]{3})),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// invalid regex
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_1() {
    auto mgr = Test_ParseFilters("frames files{((.001,.NNN)),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_2() {
    auto mgr = Test_ParseFilters("frames files(.frames){(([.][0-9]{3})),.frames}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.frames)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".frames") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Collection_Regex_3() {
    auto mgr = Test_ParseFilters("frames files(.cpp,.hpp){(([.][0-9]{3})),.cpp,.hpp}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "frames files(.cpp,.hpp)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".hpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
//// ParseFilters // Divers Tests ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#pragma region Filters Divers Tests

/* Rules
0) a filter must have 2 chars mini and the first must be a .
1) a regex must be in (( and ))
2) a , will separate filters except if between a ( and )
3) name{filter1, filter2} is a spetial form for collection filters
3.1) the name can be composed of what you want except { and }
3.2) the filter can be a regex
4) the filters cannot integrate these chars '(' ')' '{' '}' except for a regex with respect to rule 1)
5) the filters cannot integrate a ','
6) all spaces in filters will be shrinked, the best you can do is avoid them
*/

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_0() {
    auto mgr = Test_ParseFilters(
        "\
All files{.*},\
Frames Format 0(.001,.NNN){(([.][0-9]{3}))},\
Frames Format 1(XXX.png){(([0-9]{3}.png))},\
Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},\
Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},\
.md\
");

    if (mgr.prParsedFilters.size() != 6U) return false;
    if (mgr.prParsedFilters[0].filter != "All files") return false;
    if (mgr.prParsedFilters[0].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".*") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != "Frames Format 0(.001,.NNN)") return false;
    if (mgr.prParsedFilters[1].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[1].collectionfilters.find("(([.][0-9]{3}))") == mgr.prParsedFilters[1].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[2].filter != "Frames Format 1(XXX.png)") return false;
    if (mgr.prParsedFilters[2].collectionfilters.size() != 1U) return false;
    if (mgr.prParsedFilters[2].collectionfilters.find("(([0-9]{3}.png))") == mgr.prParsedFilters[2].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].filter != "Source files (*.cpp *.h *.hpp)") return false;
    if (mgr.prParsedFilters[3].collectionfilters.size() != 3U) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".cpp") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".h") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[3].collectionfilters.find(".hpp") == mgr.prParsedFilters[3].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[4].filter != "Image files (*.png *.gif *.jpg *.jpeg)") return false;
    if (mgr.prParsedFilters[4].collectionfilters.size() != 4U) return false;
    if (mgr.prParsedFilters[4].collectionfilters.find(".png") == mgr.prParsedFilters[4].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[4].collectionfilters.find(".gif") == mgr.prParsedFilters[4].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[4].collectionfilters.find(".jpg") == mgr.prParsedFilters[4].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[4].collectionfilters.find(".jpeg") == mgr.prParsedFilters[4].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[5].filter != ".md") return false;
    if (mgr.prParsedFilters[5].collectionfilters.size() != 0U) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_1() {
    auto mgr = Test_ParseFilters("Regex Custom*.h{((Custom.+[.]h))}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "Regex Custom*.h") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find("((Custom.+[.]h))") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_2() {
    auto mgr = Test_ParseFilters("C++ File (*.cpp){.cpp}");

    if (mgr.prParsedFilters.size() != 1U) return false;
    if (mgr.prParsedFilters[0].filter != "C++ File (*.cpp)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;

    return true;
}

// must be ok
bool Test_FilterManager_ParseFilters_Filters_Divers_3() {
    auto mgr = Test_ParseFilters("C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}");

    if (mgr.prParsedFilters.size() != 2U) return false;
    if (mgr.prParsedFilters[0].filter != "C/C++ File (*.c *.cpp)") return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".c") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[0].collectionfilters.find(".cpp") == mgr.prParsedFilters[0].collectionfilters.end()) return false;
    if (mgr.prParsedFilters[1].filter != " Header File (*.h)") return false;
    if (mgr.prParsedFilters[1].collectionfilters.find(".h") == mgr.prParsedFilters[1].collectionfilters.end()) return false;

    return true;
}
#pragma endregion

/////////////////////////////////////////////////////////////////////////////////////////
//// ReplaceExtentionWithCurrentFilter //////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_0() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto");
    if (res != "toto.cpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_1() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.cpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_2() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".code.cpp";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto");
    if (res != "toto.code.cpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_3() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".code.cpp";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.code.cpp") return false;

    return true;
}

// if regex, the function have no impact
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_4() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter       = "(.*\\.a\\.b)";
    mgr.prSelectedFilter.filter_regex = "(.*\\.a\\.b)";
    auto res                          = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.c") return false;

    return true;
}

// many filter in the current collection, no change
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_5() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters("C/C++ File (*.c,*.cpp){.c,.cpp}");
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.c") return false;

    return true;
}

// one filter in the current collection => change
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_6() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters("C/C++ File (*.cpp){.cpp}");
    auto res = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.cpp") return false;

    return true;
}

// .* => no change
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_7() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters(".*");
    auto res = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.c") return false;

    return true;
}

// one filter .* in one collection => no change
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_8() {
    IGFD::FilterManager mgr;
    mgr.ParseFilters("All files{.*}");
    auto res = mgr.ReplaceExtentionWithCurrentFilter("toto.c");
    if (res != "toto.c") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_9() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c.r.x");
    if (res != "toto.c.r.cpp") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_10() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp.tv";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c.r.x");
    if (res != "toto.c.cpp.tv") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_11() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = "";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.c.r.x");
    if (res != "toto.c.r.x") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_12() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp.tv";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto.");
    if (res != "toto.cpp.tv") return false;

    return true;
}

// must be ok
bool Test_FilterManager_ReplaceExtentionWithCurrentFilter_13() {
    IGFD::FilterManager mgr;
    mgr.prSelectedFilter.filter = ".cpp.tv";
    auto res                    = mgr.ReplaceExtentionWithCurrentFilter("toto");
    if (res != "toto.cpp.tv") return false;

    return true;
}

////////////////////////////////////////////////////////////////////////////
//// ENTRY POINT ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#define IfTestExist(v) vTest == std::string(v)
bool Test_FilterManager(const std::string& vTest) {
    if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::0"))
        return Test_FilterManager_ParseFilters_Filters_Simple_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::1"))
        return Test_FilterManager_ParseFilters_Filters_Simple_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::2"))
        return Test_FilterManager_ParseFilters_Filters_Simple_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::3"))
        return Test_FilterManager_ParseFilters_Filters_Simple_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::4"))
        return Test_FilterManager_ParseFilters_Filters_Simple_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::5"))
        return Test_FilterManager_ParseFilters_Filters_Simple_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::6"))
        return Test_FilterManager_ParseFilters_Filters_Simple_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::7"))
        return Test_FilterManager_ParseFilters_Filters_Simple_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::8"))
        return Test_FilterManager_ParseFilters_Filters_Simple_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::0"))
        return Test_FilterManager_ParseFilters_Filters_Collection_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::1"))
        return Test_FilterManager_ParseFilters_Filters_Collection_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::2"))
        return Test_FilterManager_ParseFilters_Filters_Collection_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::3"))
        return Test_FilterManager_ParseFilters_Filters_Collection_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::4"))
        return Test_FilterManager_ParseFilters_Filters_Collection_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::5"))
        return Test_FilterManager_ParseFilters_Filters_Collection_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::6"))
        return Test_FilterManager_ParseFilters_Filters_Collection_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::7"))
        return Test_FilterManager_ParseFilters_Filters_Collection_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::8"))
        return Test_FilterManager_ParseFilters_Filters_Collection_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::9"))
        return Test_FilterManager_ParseFilters_Filters_Collection_9();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::10"))
        return Test_FilterManager_ParseFilters_Filters_Collection_10();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::11"))
        return Test_FilterManager_ParseFilters_Filters_Collection_11();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::0"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::1"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::2"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::3"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::4"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_4();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::5"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_5();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::6"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_6();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::7"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_7();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::8"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_8();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::9"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_9();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::10"))
        return Test_FilterManager_ParseFilters_Filters_Simple_Regex_10();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::0"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::1"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::2"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::3"))
        return Test_FilterManager_ParseFilters_Filters_Collection_Regex_3();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::0"))
        return Test_FilterManager_ParseFilters_Filters_Divers_0();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::1"))
        return Test_FilterManager_ParseFilters_Filters_Divers_1();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::2"))
        return Test_FilterManager_ParseFilters_Filters_Divers_2();
    else if (IfTestExist("IGFD::FilterManager::ParseFilters::Filters::Divers::3"))
        return Test_FilterManager_ParseFilters_Filters_Divers_3();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::0"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_0();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::1"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_1();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::2"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_2();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::3"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_3();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::4"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_4();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::5"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_5();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::6"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_6();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::7"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_7();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::8"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_8();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::9"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_9();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::10"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_10();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::11"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_11();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::12"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_12();
    else if (IfTestExist("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::13"))
        return Test_FilterManager_ReplaceExtentionWithCurrentFilter_13();

    assert(0);

    return false;
}