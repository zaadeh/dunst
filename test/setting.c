#include "../src/settings.h"
#include "../src/option_parser.h"
#include "../src/settings_data.h"

#include "greatest.h"

extern const char *base;

// In this suite a few dunstrc's are tested to see if the settings code works
// This file is called setting.c, since the name settings.c caused issues.

char *test_paths[2] = {0};

TEST test_dunstrc_markup(void) {
        settings_free(&settings);

        test_paths[0] = g_strconcat(base, "/data/dunstrc.markup", NULL);
        load_settings(test_paths);

        ASSERT_STR_EQ(settings.font, "Monospace 8");


        const char *e_format = "<b>%s</b>\\n%b"; // escape the \n since it would otherwise result in the newline character
        const struct rule * r = get_rule("global");
        const char *got_format = r->format;
        ASSERT_STR_EQ(e_format, got_format);
        ASSERT(settings.indicate_hidden);

        g_clear_pointer(&test_paths[0], g_free);
        PASS();
}

TEST test_dunstrc_nomarkup(void) {
        settings_free(&settings);

        test_paths[0] = g_strconcat(base, "/data/dunstrc.nomarkup", NULL);
        load_settings(test_paths);

        ASSERT_STR_EQ(settings.font, "Monospace 8");


        const char *e_format = "<b>%s</b>\\n<i>%b</i>"; // escape the \n since it would otherwise result in the newline character
        const struct rule * r = get_rule("global");
        const char *got_format = r->format;
        ASSERT_STR_EQ(e_format, got_format);
        ASSERT(settings.indicate_hidden);

        g_clear_pointer(&test_paths[0], g_free);
        PASS();
}

// Test if the defaults in code and in dunstrc match
TEST test_dunstrc_defaults(void) {
        struct settings s_old = settings;
        struct settings s_default;
        struct settings s_dunstrc;

        test_paths[0] = g_strconcat(base, "/data/dunstrc.default", NULL);
        set_defaults();
        s_default = settings;

        load_settings(test_paths);
        s_dunstrc = settings;

        ASSERT_EQ(s_default.corner_radius, s_dunstrc.corner_radius);
        char message[500];

        for (size_t i = 0; i < G_N_ELEMENTS(allowed_settings); i++) {
                if (!allowed_settings[i].value) {
                        continue; // it's a rule, that's harder to test
                }
                if (allowed_settings[i].different_default) {
                        continue; // Skip testing, since it's an intended difference.
                }
                size_t offset = (char*)allowed_settings[i].value - (char*)&settings;
                enum setting_type type = allowed_settings[i].type;
                snprintf(message, 500, "The default of setting %s does not match. Different defaults are set in code and dunstrc"
                                , allowed_settings[i].name);
                switch (type) {
                        case TYPE_CUSTOM:
                                if (allowed_settings[i].parser == string_parse_bool) {
                                        {
                                                bool a = *(bool*) ((char*) &s_default + offset);
                                                bool b = *(bool*) ((char*) &s_dunstrc + offset);
                                                ASSERT_EQm(message, a, b);
                                        }
                                        break;
                                } else if (allowed_settings[i].parser == string_parse_maybe_int) {
                                        // not a number
                                        break;
                                }
                                break;
                        case TYPE_TIME:
                        case TYPE_INT:
                                {
                                            int a = *(int*) ((char*) &s_default + offset);
                                            int b = *(int*) ((char*) &s_dunstrc + offset);
                                            ASSERT_EQm(message, a, b);
                                }
                                break;
                        case TYPE_DOUBLE:
                        case TYPE_STRING:
                        case TYPE_PATH:
                        case TYPE_LIST:
                        case TYPE_LENGTH:
                        case TYPE_COLOR:
                        case TYPE_GRADIENT:
                                    break; // TODO implement these checks as well
                        default:
                                    printf("Type unknown %s:%d\n", __FILE__, __LINE__);
                }
                /* printf("%zu\n", offset); */
        }

        // NOTE: The last loaded settings should not be freed here
        settings_free(&s_old);
        settings_free(&s_default);

        g_clear_pointer(&test_paths[0], g_free);
        PASS();
}

SUITE(suite_setting) {
        RUN_TEST(test_dunstrc_markup);
        RUN_TEST(test_dunstrc_nomarkup);
        RUN_TEST(test_dunstrc_defaults);
}
