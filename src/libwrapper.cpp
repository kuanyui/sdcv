/*
 * This file part of sdcv - console version of Stardict program
 * http://sdcv.sourceforge.net
 * Copyright (C) 2005-2006 Evgeniy <dushistov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>
#include <map>
#include <memory>
#include <algorithm>

#include <glib/gi18n.h>

#include "utils.hpp"

#include "libwrapper.hpp"

namespace {
namespace ESC {
const char end[]       = "\033[0m";
const char bold[]      = "\033[1m";
const char italic[]    = "\033[3m";
const char black[]     = "\033[0;30m";
const char red[]       = "\033[0;31m";
const char green[]     = "\033[0;32m";
const char yellow[]    = "\033[0;33m";
const char blue[]      = "\033[0;34m";
const char magenta[]   = "\033[0;35m";
const char cyan[]      = "\033[0;36m";
const char white[]     = "\033[0;37m";
const char b_black[]   = "\033[0;90m";
const char b_red[]     = "\033[0;91m";
const char b_green[]   = "\033[0;92m";
const char b_yellow[]  = "\033[0;93m";
const char b_blue[]    = "\033[0;94m";
const char b_magenta[] = "\033[0;95m";
const char b_cyan[]    = "\033[0;96m";
const char b_white[]   = "\033[0;97m";
}

namespace FMT {
const char *search_term   = ESC::bold;
const char *name_of_dict  = ESC::b_blue;
const char *transcription = ESC::bold;
const char *example       = ESC::white;
const char *kref          = ESC::bold;
const char *abr           = ESC::green;
const char *hr            = ESC::b_black;
}

std::string xdxf2text(const char *p, bool colorize_output)
{
    std::string res;
    for (; *p; ++p) {
        if (*p != '<') {
            if (g_str_has_prefix(p, "&gt;")) {
                res += ">";
                p += 3;
            } else if (g_str_has_prefix(p, "&lt;")) {
                res += "<";
                p += 3;
            } else if (g_str_has_prefix(p, "&amp;")) {
                res += "&";
                p += 4;
            } else if (g_str_has_prefix(p, "&quot;")) {
                res += "\"";
                p += 5;
            } else if (g_str_has_prefix(p, "&apos;")) {
                res += "\'";
                p += 5;
            } else
                res += *p;
            continue;
        }

        const char *next = strchr(p, '>');
        if (!next)
            continue;

        const std::string name(p + 1, next - p - 1);
        if (name == "abr")
            res += colorize_output ? FMT::abr : "";
        else if (name == "/abr")
            res += colorize_output ? ESC::end : "";
        else if (name == "k") {
            const char *begin = next;
            if ((next = strstr(begin, "</k>")) != nullptr)
                next += sizeof("</k>") - 1 - 1;
            else
                next = begin;
        } else if (name == "kref") {
            res += colorize_output ? FMT::kref : "";
        } else if (name == "/kref") {
            res += colorize_output ? ESC::end : "";
        } else if (name == "b")
            res += colorize_output ? ESC::bold : "";
        else if (name == "/b")
            res += colorize_output ? ESC::end : "";
        else if (name == "i")
            res += colorize_output ? ESC::italic : "";
        else if (name == "/i")
            res += colorize_output ? ESC::end : "";
        else if (name == "tr") {
            if (colorize_output)
                res += FMT::transcription;
            res += "[";
        } else if (name == "/tr") {
            res += "]";
            if (colorize_output)
                res += ESC::end;
        } else if (name == "ex")
            res += colorize_output ? FMT::example : "";
        else if (name == "/ex")
            res += colorize_output ? ESC::end : "";
        else if (!name.empty() && name[0] == 'c' && name != "co") {
            std::string::size_type pos = name.find("code");
            if (pos != std::string::npos) {
                pos += sizeof("code=\"") - 1;
                std::string::size_type end_pos = name.find("\"");
                const std::string color(name, pos, end_pos - pos);
                res += "";
            } else {
                res += "";
            }
        } else if (name == "/c")
            res += "";

        p = next;
    }
    return res;
}

std::string html2text(const char *p, bool colorize_output)
{
    std::string res;
    for (; *p; ++p) {
        if (*p != '<') {
            if (g_str_has_prefix(p, "&gt;")) {
                res += ">";
                p += 3;
            } else if (g_str_has_prefix(p, "&lt;")) {
                res += "<";
                p += 3;
            } else if (g_str_has_prefix(p, "&amp;")) {
                res += "&";
                p += 4;
            } else if (g_str_has_prefix(p, "&quot;")) {
                res += "\"";
                p += 5;
            } else if (g_str_has_prefix(p, "&apos;")) {
                res += "\'";
                p += 5;
            } else
                res += *p;
            continue;
        }

        const char *next = strchr(p, '>');
        if (!next)
            continue;

        std::string name(p + 1, next - p - 1);
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.compare(0, 4, "font") == 0) {
            res += colorize_output ? FMT::abr : "";
        } else if (name.compare(0, 5, "/font") == 0) {
            res += colorize_output ? ESC::end : "";
        } else if (name.compare(0, 2, "br") == 0) {
            res += "\n";
        } else if (name == "kref") {
            res += colorize_output ? FMT::kref : "";
        } else if (name == "/kref") {
            res += colorize_output ? ESC::end : "";
        } else if (name == "b") {
            res += colorize_output ? ESC::bold : "";
        } else if (name == "/b") {
            res += colorize_output ? ESC::end : "";
        } else if (name == "i" || name == "I") {
            res += colorize_output ? ESC::italic : "";
        } else if (name == "/i") {
            res += colorize_output ? ESC::end : "";
        } else if (name == "tr") {
            if (colorize_output) { res += FMT::transcription; }
            res += "[";
        } else if (name == "/tr") {
            res += "]";
            if (colorize_output) { res += ESC::end; }
        } else if (name.compare(0, 2, "hr") == 0) {
            if (colorize_output) { res += FMT::hr; }
            res += "\n--------------------\n";
            if (colorize_output) { res += ESC::end; }
        }

        p = next;
    }
    return res;
}

std::string parse_data(const gchar *data, bool colorize_output)
{
    if (!data)
        return "";

    std::string res;
    guint32 data_size, sec_size = 0;
    gchar *m_str;
    const gchar *p = data;
    data_size = get_uint32(p);
    p += sizeof(guint32);
    while (guint32(p - data) < data_size) {
        switch (*p++) {
        case 'h': // HTML data
            sec_size = static_cast<guint32>(strlen(p));
            if (sec_size) {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += html2text(m_str, colorize_output);
                g_free(m_str);
            }
            sec_size++;
            break;
        case 'w': // WikiMedia markup data
        case 'm': // plain text, utf-8
        case 'l': // not utf-8, some other locale encoding, discouraged, need more work...
            sec_size = static_cast<guint32>(strlen(p));
            if (sec_size) {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += m_str;
                g_free(m_str);
            }
            sec_size++;
            break;
        case 'g': // pango markup data
        case 'x': // xdxf
            sec_size = static_cast<guint32>(strlen(p));
            if (sec_size) {
                res += "\n";
                m_str = g_strndup(p, sec_size);
                res += xdxf2text(m_str, colorize_output);
                g_free(m_str);
            }
            sec_size++;
            break;
        case 't': // english phonetic string
            sec_size = static_cast<guint32>(strlen(p));
            if (sec_size) {
                res += "\n";
                if (colorize_output)
                    res += FMT::transcription;
                res += "[" + std::string(p, sec_size) + "]";
                if (colorize_output)
                    res += ESC::end;
            }
            sec_size++;
            break;
        case 'k': // KingSoft PowerWord data
        case 'y': // chinese YinBiao or japanese kana, utf-8
            sec_size = static_cast<guint32>(strlen(p));
            if (sec_size)
                res += std::string(p, sec_size);
            sec_size++;
            break;
        case 'W': // wav file
        case 'P': // picture data
            sec_size = get_uint32(p);
            sec_size += sizeof(guint32);
            break;
        }
        p += sec_size;
    }

    return res;
}
}
void Library::SimpleLookup(const std::string &str, TSearchResultList &res_list)
{
    glong ind;
    res_list.reserve(ndicts());
    for (gint idict = 0; idict < ndicts(); ++idict)
        if (SimpleLookupWord(str.c_str(), ind, idict))
            res_list.push_back(
                TSearchResult(dict_name(idict),
                              poGetWord(ind, idict),
                              parse_data(poGetWordData(ind, idict), colorize_output_)));
}

void Library::LookupWithFuzzy(const std::string &str, TSearchResultList &res_list)
{
    static const int MAXFUZZY = 10;

    gchar *fuzzy_res[MAXFUZZY];
    if (!Libs::LookupWithFuzzy(str.c_str(), fuzzy_res, MAXFUZZY))
        return;

    for (gchar **p = fuzzy_res, **end = (fuzzy_res + MAXFUZZY); p != end && *p; ++p) {
        SimpleLookup(*p, res_list);
        g_free(*p);
    }
}

void Library::LookupWithRule(const std::string &str, TSearchResultList &res_list)
{
    std::vector<gchar *> match_res((MAX_MATCH_ITEM_PER_LIB)*ndicts());

    const gint nfound = Libs::LookupWithRule(str.c_str(), &match_res[0]);
    if (nfound == 0)
        return;

    for (gint i = 0; i < nfound; ++i) {
        SimpleLookup(match_res[i], res_list);
        g_free(match_res[i]);
    }
}

void Library::LookupData(const std::string &str, TSearchResultList &res_list)
{
    std::vector<std::vector<gchar *>> drl(ndicts());
    if (!Libs::LookupData(str.c_str(), &drl[0]))
        return;
    for (int idict = 0; idict < ndicts(); ++idict)
        for (gchar *res : drl[idict]) {
            SimpleLookup(res, res_list);
            g_free(res);
        }
}

void Library::print_search_result(FILE *out, const TSearchResult &res, bool &first_result)
{
    std::string loc_bookname, loc_def, loc_exp;

    if (!utf8_output_) {
        loc_bookname = utf8_to_locale_ign_err(res.bookname);
        loc_def = utf8_to_locale_ign_err(res.def);
        loc_exp = utf8_to_locale_ign_err(res.exp);
    }
    if (json_) {
        if (!first_result) {
            fputs(",", out);
        } else {
            first_result = false;
        }
        fprintf(out, "{\"dict\": \"%s\",\"word\":\"%s\",\"definition\":\"%s\"}",
                json_escape_string(res.bookname).c_str(),
                json_escape_string(res.def).c_str(),
                json_escape_string(res.exp).c_str());

    } else {
        fprintf(out,
                "-->%s%s%s\n"
                "-->%s%s%s\n"
                "%s\n\n",
                colorize_output_ ? FMT::name_of_dict : "",
                utf8_output_ ? res.bookname.c_str() : loc_bookname.c_str(),
                colorize_output_ ? ESC::end : "",
                colorize_output_ ? FMT::search_term : "",
                utf8_output_ ? res.def.c_str() : loc_def.c_str(),
                colorize_output_ ? ESC::end : "",
                utf8_output_ ? res.exp.c_str() : loc_exp.c_str());
    }
}

namespace
{
class sdcv_pager final
{
public:
    explicit sdcv_pager(bool ignore_env = false)
    {
        output = stdout;
        if (ignore_env) {
            return;
        }
        const gchar *pager = g_getenv("SDCV_PAGER");
        if (pager && (output = popen(pager, "w")) == nullptr) {
            perror(_("popen failed"));
            output = stdout;
        }
    }
    sdcv_pager(const sdcv_pager &) = delete;
    sdcv_pager &operator=(const sdcv_pager &) = delete;
    ~sdcv_pager()
    {
        if (output != stdout) {
            pclose(output);
        }
    }
    FILE *get_stream() { return output; }

private:
    FILE *output;
};
}

bool Library::process_phrase(const char *loc_str, IReadLine &io, bool force)
{
    if (nullptr == loc_str)
        return true;

    std::string query;

    analyze_query(loc_str, query);
    if (!query.empty())
        io.add_to_history(query.c_str());

    gsize bytes_read;
    gsize bytes_written;
    glib::Error err;
    glib::CharStr str;
    if (!utf8_input_)
        str.reset(g_locale_to_utf8(loc_str, -1, &bytes_read, &bytes_written, get_addr(err)));
    else
        str.reset(g_strdup(loc_str));

    if (nullptr == get_impl(str)) {
        fprintf(stderr, _("Can not convert %s to utf8.\n"), loc_str);
        fprintf(stderr, "%s\n", err->message);
        return false;
    }

    if (str[0] == '\0')
        return true;

    TSearchResultList res_list;

    switch (analyze_query(get_impl(str), query)) {
    case qtFUZZY:
        LookupWithFuzzy(query, res_list);
        break;
    case qtREGEXP:
        LookupWithRule(query, res_list);
        break;
    case qtSIMPLE:
        SimpleLookup(get_impl(str), res_list);
        if (res_list.empty() && fuzzy_)
            LookupWithFuzzy(get_impl(str), res_list);
        break;
    case qtDATA:
        LookupData(query, res_list);
        break;
    default:
        /*nothing*/;
    }

    bool first_result = true;
    if (json_) {
        fputc('[', stdout);
    }
    if (!res_list.empty()) {
        /* try to be more clever, if there are
		   one or zero results per dictionary show all
		*/
        bool show_all_results = true;
        typedef std::map<std::string, int, std::less<std::string>> DictResMap;
        if (!force) {
            DictResMap res_per_dict;
            for (const TSearchResult &search_res : res_list) {
                auto r = res_per_dict.equal_range(search_res.bookname);
                DictResMap tmp(r.first, r.second);
                if (tmp.empty()) //there are no yet such bookname in map
                    res_per_dict.insert(DictResMap::value_type(search_res.bookname, 1));
                else {
                    ++((tmp.begin())->second);
                    if (tmp.begin()->second > 1) {
                        show_all_results = false;
                        break;
                    }
                }
            }
        } //if (!force)

        if (!show_all_results && !force) {
            if (!json_) {
                printf(_("Found %zu items, similar to %s.\n"), res_list.size(),
                       utf8_output_ ? get_impl(str) : utf8_to_locale_ign_err(get_impl(str)).c_str());
            }
            for (size_t i = 0; i < res_list.size(); ++i) {
                const std::string loc_bookname = utf8_to_locale_ign_err(res_list[i].bookname);
                const std::string loc_def = utf8_to_locale_ign_err(res_list[i].def);
                printf("%zu)%s%s%s-->%s%s%s\n", i,
                       colorize_output_ ? FMT::name_of_dict : "",
                       utf8_output_ ? res_list[i].bookname.c_str() : loc_bookname.c_str(),
                       colorize_output_ ? ESC::end : "",
                       colorize_output_ ? FMT::search_term : "",
                       utf8_output_ ? res_list[i].def.c_str() : loc_def.c_str(),
                       colorize_output_ ? ESC::end : "");
            }
            int choise;
            std::unique_ptr<IReadLine> choice_readline(create_readline_object());
            for (;;) {
                std::string str_choise;
                choice_readline->read(_("Your choice[-1 to abort]: "), str_choise);
                sscanf(str_choise.c_str(), "%d", &choise);
                if (choise >= 0 && choise < int(res_list.size())) {
                    sdcv_pager pager;
                    io.add_to_history(res_list[choise].def.c_str());
                    print_search_result(pager.get_stream(), res_list[choise], first_result);
                    break;
                } else if (choise == -1) {
                    break;
                } else
                    printf(_("Invalid choice.\nIt must be from 0 to %zu or -1.\n"),
                           res_list.size() - 1);
            }
        } else {
            sdcv_pager pager(force || json_);
            if (!json_) {
                fprintf(pager.get_stream(), _("Found %zu items, similar to %s.\n"),
                        res_list.size(), utf8_output_ ? get_impl(str) : utf8_to_locale_ign_err(get_impl(str)).c_str());
            }
            for (const TSearchResult &search_res : res_list) {
                print_search_result(pager.get_stream(), search_res, first_result);
            }
        }

    } else {
        std::string loc_str;
        if (!utf8_output_)
            loc_str = utf8_to_locale_ign_err(get_impl(str));
        if (!json_)
            printf(_("Nothing similar to %s, sorry :(\n"), utf8_output_ ? get_impl(str) : loc_str.c_str());
    }

    if (json_) {
        fputs("]\n", stdout);
    }
    return true;
}
