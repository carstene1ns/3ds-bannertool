
#include <cstring>
#include <algorithm>
#include <map>
#include <vector>

#include "types.h"
#include "log.h"
#include "utils.h"
#include "3ds/cbmd.h"

static ArgMap cmd_get_args(int argc, char* argv[]) {
	ArgMap args;
	for(int i = 0; i < argc; i++) {
		if((strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) && argc != i + 1) {
			args[argv[i]] = argv[i + 1];
			i++;
		}
	}

	return args;
}

static std::string cmd_find_arg(const ArgMap &args, const std::string& shortOpt,
                                const std::string& longOpt, const std::string& def) {
	std::string sopt = "-" + shortOpt;
	std::string lopt = "--" + longOpt;

	auto match = args.find(sopt);
	if(match != args.end()) {
		return (*match).second;
	}

	match = args.find(lopt);
	if(match != args.end()) {
		return (*match).second;
	}

	return def;
}

static std::vector<std::string> cmd_parse_list(const std::string& list) {
	std::vector<std::string> ret;
	std::string::size_type lastPos = 0;
	std::string::size_type pos = 0;
	while((pos = list.find(',', lastPos)) != std::string::npos) {
		ret.push_back(list.substr(lastPos, pos - lastPos));
		lastPos = pos + 1;
	}

	if(lastPos < list.size()) {
		ret.push_back(list.substr(lastPos));
	}

	return ret;
}

static const char* help_makebanner = R"(makebanner - Creates a .bnr file.
  -i/--image:                  Optional if specified for a language or with a CGFX. PNG file to use as the banner's default image.
    -eei/--eurenglishimage:    Optional if default or CGFX specified. PNG file to use as the banner's EUR English image.
    -efi/--eurfrenchimage:     Optional if default or CGFX specified. PNG file to use as the banner's EUR French image.
    -egi/--eurgermanimage:     Optional if default or CGFX specified. PNG file to use as the banner's EUR German image.
    -eii/--euritalianimage:    Optional if default or CGFX specified. PNG file to use as the banner's EUR Italian image.
    -esi/--eurspanishimage:    Optional if default or CGFX specified. PNG file to use as the banner's EUR Spanish image.
    -edi/--eurdutchimage:      Optional if default or CGFX specified. PNG file to use as the banner's EUR Dutch image.
    -epi/--eurportugueseimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Portuguese image.
    -eri/--eurrussianimage:    Optional if default or CGFX specified. PNG file to use as the banner's EUR Russian image.
    -jji/--jpnjapaneseimage:   Optional if default or CGFX specified. PNG file to use as the banner's JPN Japanese image.
    -uei/--usaenglishimage:    Optional if default or CGFX specified. PNG file to use as the banner's USA English image.
    -ufi/--usafrenchimage:     Optional if default or CGFX specified. PNG file to use as the banner's USA French image.
    -usi/--usaspanishimage:    Optional if default or CGFX specified. PNG file to use as the banner's USA Spanish image.
    -upi/--usaportugueseimage: Optional if default or CGFX specified. PNG file to use as the banner's USA Portuguese image.

  -ci/--cgfximage:                  Optional if specified for a language or with a PNG. CGFX file to use as the banner's default image.
    -eeci/--eurenglishcgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's EUR English image.
    -efci/--eurfrenchcgfximage:     Optional if default or PNG specified. CGFX file to use as the banner's EUR French image.
    -egci/--eurgermancgfximage:     Optional if default or PNG specified. CGFX file to use as the banner's EUR German image.
    -eici/--euritaliancgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's EUR Italian image.
    -esci/--eurspanishcgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's EUR Spanish image.
    -edci/--eurdutchcgfximage:      Optional if default or PNG specified. CGFX file to use as the banner's EUR Dutch image.
    -epci/--eurportuguesecgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Portuguese image.
    -erci/--eurrussiancgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's EUR Russian image.
    -jjci/--jpnjapanesecgfximage:   Optional if default or PNG specified. CGFX file to use as the banner's JPN Japanese image.
    -ueci/--usaenglishcgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's USA English image.
    -ufci/--usafrenchcgfximage:     Optional if default or PNG specified. CGFX file to use as the banner's USA French image.
    -usci/--usaspanishcgfximage:    Optional if default or PNG specified. CGFX file to use as the banner's USA Spanish image.
    -upci/--usaportuguesecgfximage: Optional if default or PNG specified. CGFX file to use as the banner's USA Portuguese image.

  -a/--audio:      Optional if with a CWAV.    WAV/OGG file to use as the banner's tune.
  -ca/--cwavaudio: Optional if with a WAV/OGG. CWAV file to use as the banner's tune.
  -o/--output:                                 File to output the created banner to.
)";

static const char* help_makesmdh = R"(makesmdh - Creates a .smdh/.icn file.
  -s/--shorttitle:                                  Default short title of the application.
    -js/--japaneseshorttitle:            [Optional] Japanese short title of the application.
    -es/--englishshorttitle:             [Optional] English short title of the application.
    -fs/--frenchshorttitle:              [Optional] French short title of the application.
    -gs/--germanshorttitle:              [Optional] German short title of the application.
    -is/--italianshorttitle:             [Optional] Italian short title of the application.
    -ss/--spanishshorttitle:             [Optional] Spanish short title of the application.
    -scs/--simplifiedchineseshorttitle:  [Optional] Simplified Chinese short title of the application.
    -ks/--koreanshorttitle:              [Optional] Korean short title of the application.
    -ds/--dutchshorttitle:               [Optional] Dutch short title of the application.
    -ps/--portugueseshorttitle:          [Optional] Portuguese short title of the application.
    -rs/--russianshorttitle:             [Optional] Russian short title of the application.
    -tcs/--traditionalchineseshorttitle: [Optional] Traditional Chinese short title of the application.

  -l/--longtitle:                                  Default long title of the application.
    -jl/--japaneselongtitle:            [Optional] Japanese long title of the application.
    -el/--englishlongtitle:             [Optional] English long title of the application.
    -fl/--frenchlongtitle:              [Optional] French long title of the application.
    -gl/--germanlongtitle:              [Optional] German long title of the application.
    -il/--italianlongtitle:             [Optional] Italian long title of the application.
    -sl/--spanishlongtitle:             [Optional] Spanish long title of the application.
    -scl/--simplifiedchineselongtitle:  [Optional] Simplified Chinese long title of the application.
    -kl/--koreanlongtitle:              [Optional] Korean long title of the application.
    -dl/--dutchlongtitle:               [Optional] Dutch long title of the application.
    -pl/--portugueselongtitle:          [Optional] Portuguese long title of the application.
    -rl/--russianlongtitle:             [Optional] Russian long title of the application.
    -tcl/--traditionalchineselongtitle: [Optional] Traditional Chinese long title of the application.

  -p/--publisher:                                  Default publisher of the application.
    -jp/--japanesepublisher:            [Optional] Japanese publisher of the application.
    -ep/--englishpublisher:             [Optional] English publisher of the application.
    -fp/--frenchpublisher:              [Optional] French publisher of the application.
    -gp/--germanpublisher:              [Optional] German publisher of the application.
    -ip/--italianpublisher:             [Optional] Italian publisher of the application.
    -sp/--spanishpublisher:             [Optional] Spanish publisher of the application.
    -scp/--simplifiedchinesepublisher:  [Optional] Simplified Chinese publisher of the application.
    -kp/--koreanpublisher:              [Optional] Korean publisher of the application.
    -dp/--dutchpublisher:               [Optional] Dutch publisher of the application.
    -pp/--portuguesepublisher:          [Optional] Portuguese publisher of the application.
    -rp/--russianpublisher:             [Optional] Russian publisher of the application.
    -tcp/--traditionalchinesepublisher: [Optional] Traditional Chinese publisher of the application.

  -i/--icon:                            PNG file to use as an icon.
  -o/--output:                          File to output the created SMDH/ICN to.
  -r/--regions:              [Optional] Comma separated list of regions to lock the SMDH to.
     Valid regions: regionfree, japan, northamerica, europe, australia, china, korea, taiwan.

  -f/--flags:                [Optional] Flags to apply to the SMDH file.
     Valid flags: visible, autoboot, allow3d, requireeula, autosave, extendedbanner,
                  ratingrequired, savedata, recordusage, nosavebackups, new3ds.

  -mmid/--matchmakerid:      [Optional] Match maker ID of the SMDH.
  -mmbid/--matchmakerbitid:  [Optional] Match maker BIT ID of the SMDH.
  -ev/--eulaversion:         [Optional] Version of the EULA required to be accepted before launching.
  -obf/--optimalbannerframe: [Optional] Optimal frame of the accompanying banner.
  -spid/--streetpassid:      [Optional] Streetpass ID of the SMDH.
  -cer/--cero:               [Optional] CERO rating number (0-255).
  -er/--esrb:                [Optional] ESRB rating number (0-255).
  -ur/--usk:                 [Optional] USK rating number (0-255).
  -pgr/--pegigen:            [Optional] PEGI GEN rating number (0-255).
  -ppr/--pegiptr:            [Optional] PEGI PTR rating number (0-255).
  -pbr/--pegibbfc:           [Optional] PEGI BBFC rating number (0-255).
  -cr/--cob:                 [Optional] COB rating number (0-255).
  -gr/--grb:                 [Optional] GR rating number (0-255).
  -cgr/--cgsrr:              [Optional] CGSRR rating number (0-255).
)";

static const char* help_makecwav = R"(makecwav - Creates a CWAV file from a WAV/OGG.
  -i/--input:                     WAV/OGG file to convert.
  -o/--output:                    File to output the created CWAV to.
  -l/--loop:           [Optional] Whether or not the audio should loop (false/true).
  -s/--loopstartframe: [Optional] Sample frame to return to when looping.
  -f/--loopendframe:   [Optional] Sample frame to loop at.
)";

static const char* help_lz11 = R"(lz11 - Compresses a file with LZ11.
  -i/--input:  File to compress.
  -o/--output: File to output the compressed data to.
)";

static void cmd_print_info(const std::string& command) {
	if(command.compare("makebanner") == 0) {
		printf("%s", help_makebanner);
	} else if(command.compare("makesmdh") == 0) {
		printf("%s", help_makesmdh);
	} else if(command.compare("makecwav") == 0) {
		printf("%s", help_makecwav);
	} else if(command.compare("lz11") == 0) {
		printf("%s", help_lz11);
	}
}

static void cmd_print_commands() {
	printf("Available commands:\n");
	cmd_print_info("makebanner");
	printf("\n");
	cmd_print_info("makesmdh");
	printf("\n");
	cmd_print_info("makecwav");
	printf("\n");
	cmd_print_info("lz11");
}

static void cmd_print_version() {
	printf("bannertool v%s\n", VERSION);
}

static void cmd_print_usage(const std::string& executedFrom, bool shortCmds) {
	cmd_print_version();
	printf("Usage: %s <command> <args>\n", executedFrom.c_str());
	if(shortCmds)
		printf("Available commands: makebanner makesmdh makecwav lz11\n");
}

static void cmd_print_help(const std::string& executedFrom) {
	cmd_print_usage(executedFrom, false);
	cmd_print_commands();

	printf("\n--help - Show this help message\n");
	printf("--version - Show version information\n");
}

static void cmd_missing_args(const std::string& command) {
	LOG_F(ERROR, "Missing arguments for command \"%s\".\n", command.c_str());
	cmd_print_info(command);
}

static void cmd_invalid_arg(const std::string& argument, const std::string& command) {
	LOG_F(ERROR, "Invalid value for argument \"%s\" in command \"%s\".\n", argument.c_str(), command.c_str());
	cmd_print_info(command);
}

static void cmd_invalid_command(const std::string& command) {
	LOG_F(ERROR, "Invalid command \"%s\".\n", command.c_str());
	cmd_print_commands();
}

// entrypoint

int main(int argc, char* argv[]) {
	if(argc < 2) {
		cmd_print_usage(argv[0], true);
		return -1;
	}

	if(argc == 2 && strcmp(argv[1], "--help") == 0) {
		cmd_print_help(argv[0]);
		return 0;
	}

	if(argc == 2 && strcmp(argv[1], "--version") == 0) {
		cmd_print_version();
		return 0;
	}

	char* command = argv[1];
	ArgMap args = cmd_get_args(argc, argv);
	if(strcmp(command, "makebanner") == 0) {
		const fs::path audio = cmd_find_arg(args, "a", "audio", "");
		const fs::path cwavFile = cmd_find_arg(args, "ca", "cwavaudio", "");
		const fs::path output = cmd_find_arg(args, "o", "output", "");
		if((audio.empty() && cwavFile.empty()) || output.empty()) {
			cmd_missing_args(command);
			return -1;
		}

		static const char* imageShortArgs[CBMD_NUM_CGFXS] = {"i", "eei", "efi", "egi", "eii", "esi", "edi", "epi", "eri", "jji", "uei", "ufi", "usi", "upi"};
		static const char* imageLongArgs[CBMD_NUM_CGFXS] = {"image", "eurenglishimage", "eurfrenchimage", "eurgermanimage", "euritalianimage", "eurspanishimage", "eurdutchimage", "eurportugueseimage", "eurrussianimage", "jpnjapaneseimage", "usaenglishimage", "usafrenchimage", "usaspanishimage", "usaportugueseimage"};
		static const char* cgfxImageShortArgs[CBMD_NUM_CGFXS] = {"ci", "eeci", "efci", "egci", "eici", "esci", "edci", "epci", "erci", "jjci", "ueci", "ufci", "usci", "upci"};
		static const char* cgfxImageLongArgs[CBMD_NUM_CGFXS] = {"cgfximage", "eurenglishcgfximage", "eurfrenchcgfximage", "eurgermancgfximage", "euritaliancgfximage", "eurspanishcgfximage", "eurdutchcgfximage", "eurportuguesecgfximage", "eurrussiancgfximage", "jpnjapanesecgfximage", "usaenglishcgfximage", "usafrenchcgfximage", "usaspanishcgfximage", "usaportuguesecgfximage"};

		fs::path images[CBMD_NUM_CGFXS] = {""};
		fs::path cgfxFiles[CBMD_NUM_CGFXS] = {""};

		bool found = false;

		for(int i = 0; i < CBMD_NUM_CGFXS; i++) {
			images[i] = cmd_find_arg(args, imageShortArgs[i], imageLongArgs[i], "");
			cgfxFiles[i] = cmd_find_arg(args, cgfxImageShortArgs[i], cgfxImageLongArgs[i], "");

			found = found || !images[i].empty() || !cgfxFiles[i].empty();
		}

		if(!found) {
			cmd_missing_args(command);
			return -1;
		}

		return cmd_make_banner(images, audio, cgfxFiles, cwavFile, output);
	} else if(strcmp(command, "makesmdh") == 0) {
		const fs::path icon = cmd_find_arg(args, "i", "icon", "");
		const fs::path output = cmd_find_arg(args, "o", "output", "");
		if(icon.empty() || output.empty()) {
			cmd_missing_args(command);
			return -1;
		}

		SMDH smdh;
		memset(&smdh, 0, sizeof(smdh));

		memcpy(smdh.magic, SMDH_MAGIC, sizeof(smdh.magic));

		static const char* shortTitleShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"js", "es", "fs", "gs", "is", "ss", "scs", "ks", "ds", "ps", "rs", "tcs"};
		static const char* shortTitleLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japaneseshorttitle", "englishshorttitle", "frenchshorttitle", "germanshorttitle", "italianshorttitle", "spanishshorttitle", "simplifiedchineseshorttitle", "koreanshorttitle", "dutchshorttitle", "portugueseshorttitle", "russianshorttitle", "traditionalchineseshorttitle"};
		static const char* longTitleShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"jl", "el", "fl", "gl", "il", "sl", "scl", "kl", "dl", "pl", "rl", "tcl"};
		static const char* longTitleLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japaneselongtitle", "englishlongtitle", "frenchlongtitle", "germanlongtitle", "italianlongtitle", "spanishlongtitle", "simplifiedchineselongtitle", "koreanlongtitle", "dutchlongtitle", "portugueselongtitle", "russianlongtitle", "traditionalchineselongtitle"};
		static const char* publisherShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"jp", "ep", "fp", "gp", "ip", "sp", "scp", "kp", "dp", "pp", "rp", "tcp"};
		static const char* publisherLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japanesepublisher", "englishpublisher", "frenchpublisher", "germanpublisher", "italianpublisher", "spanishpublisher", "simplifiedchinesepublisher", "koreanpublisher", "dutchpublisher", "portuguesepublisher", "russianpublisher", "traditionalchinesepublisher"};

		const std::string shortTitle = cmd_find_arg(args, "s", "shorttitle", "");
		const std::string longTitle = cmd_find_arg(args, "l", "longtitle", "");
		const std::string publisher = cmd_find_arg(args, "p", "publisher", "");

		bool shortTitleFound = false;
		bool longTitleFound = false;
		bool publisherFound = false;

		for(int i = 0; i < SMDH_NUM_LANGUAGE_SLOTS; i++) {
			std::string currShortTitle = shortTitle;
			std::string currLongTitle = longTitle;
			std::string currPublisher = publisher;

			if(i < SMDH_NUM_VALID_LANGUAGE_SLOTS) {
				currShortTitle = cmd_find_arg(args, shortTitleShortArgs[i], shortTitleLongArgs[i], shortTitle);
				currLongTitle = cmd_find_arg(args, longTitleShortArgs[i], longTitleLongArgs[i], longTitle);
				currPublisher = cmd_find_arg(args, publisherShortArgs[i], publisherLongArgs[i], publisher);
			}

			shortTitleFound = shortTitleFound || !currShortTitle.empty();
			longTitleFound = longTitleFound || !currLongTitle.empty();
			publisherFound = publisherFound || !currPublisher.empty();

			if(!utf8_to_utf16(smdh.titles[i].shortTitle, currShortTitle, sizeof(smdh.titles[i].shortTitle))) {
				LOG_F(WARNING, "Truncating utf16 %s string.",
				      (i < SMDH_NUM_VALID_LANGUAGE_SLOTS ? shortTitleLongArgs[i] : "short title"));
			}
			if(!utf8_to_utf16(smdh.titles[i].longTitle, currLongTitle, sizeof(smdh.titles[i].longTitle))) {
				LOG_F(WARNING, "Truncating utf16 %s string.",
				      (i < SMDH_NUM_VALID_LANGUAGE_SLOTS ? longTitleLongArgs[i] : "long title"));
			}
			if(!utf8_to_utf16(smdh.titles[i].publisher, currPublisher, sizeof(smdh.titles[i].publisher))) {
				LOG_F(WARNING, "Truncating utf16 %s string.", (i < SMDH_NUM_VALID_LANGUAGE_SLOTS ? publisherLongArgs[i] : "publisher"));
			}
		}

		if(!shortTitleFound || !longTitleFound || !publisherFound) {
			cmd_missing_args(command);
			return -1;
		}

		std::vector<std::string> regions = cmd_parse_list(cmd_find_arg(args, "r", "regions", "regionfree"));
		for(std::vector<std::string>::iterator it = regions.begin(); it != regions.end(); it++) {
			const std::string region = *it;
			if(region.compare("regionfree") == 0) {
				smdh.settings.regionLock = SMDH_REGION_FREE;
				break;
			} else if(region.compare("japan") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_JAPAN;
			} else if(region.compare("northamerica") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_NORTH_AMERICA;
			} else if(region.compare("europe") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_EUROPE;
			} else if(region.compare("australia") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_AUSTRALIA;
			} else if(region.compare("china") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_CHINA;
			} else if(region.compare("korea") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_KOREA;
			} else if(region.compare("taiwan") == 0) {
				smdh.settings.regionLock |= SMDH_REGION_TAIWAN;
			} else {
				cmd_invalid_arg("regions", command);
				return -1;
			}
		}

		std::vector<std::string> flags = cmd_parse_list(cmd_find_arg(args, "f", "flags", "visible,allow3d,recordusage"));
		for(std::vector<std::string>::iterator it = flags.begin(); it != flags.end(); it++) {
			const std::string flag = *it;
			if(flag.compare("visible") == 0) {
				smdh.settings.flags |= SMDH_FLAG_VISIBLE;
			} else if(flag.compare("autoboot") == 0) {
				smdh.settings.flags |= SMDH_FLAG_AUTO_BOOT;
			} else if(flag.compare("allow3d") == 0) {
				smdh.settings.flags |= SMDH_FLAG_ALLOW_3D;
			} else if(flag.compare("requireeula") == 0) {
				smdh.settings.flags |= SMDH_FLAG_REQUIRE_EULA;
			} else if(flag.compare("autosave") == 0) {
				smdh.settings.flags |= SMDH_FLAG_AUTO_SAVE_ON_EXIT;
			} else if(flag.compare("extendedbanner") == 0) {
				smdh.settings.flags |= SMDH_FLAG_USE_EXTENDED_BANNER;
			} else if(flag.compare("ratingrequired") == 0) {
				smdh.settings.flags |= SMDH_FLAG_RATING_REQUIED;
			} else if(flag.compare("savedata") == 0) {
				smdh.settings.flags |= SMDH_FLAG_USE_SAVE_DATA;
			} else if(flag.compare("recordusage") == 0) {
				smdh.settings.flags |= SMDH_FLAG_RECORD_USAGE;
			} else if(flag.compare("nosavebackups") == 0) {
				smdh.settings.flags |= SMDH_FLAG_DISABLE_SAVE_BACKUPS;
			} else if(flag.compare("new3ds") == 0) {
				smdh.settings.flags |= SMDH_FLAG_NEW_3DS;
			} else {
				cmd_invalid_arg("flags", command);
				return -1;
			}
		}

		smdh.settings.matchMakerId = (u32) atoi(cmd_find_arg(args, "mmid", "matchmakerid", "0").c_str());
		smdh.settings.matchMakerBitId = (u64) atoll(cmd_find_arg(args, "mmbid", "matchmakerbitid", "0").c_str());

		smdh.settings.eulaVersion = (u16) atoi(cmd_find_arg(args, "ev", "eulaversion", "0").c_str());
		smdh.settings.optimalBannerFrame = (u32) atoi(cmd_find_arg(args, "obf", "optimalbannerframe", "0").c_str());
		smdh.settings.streetpassId = (u32) atoi(cmd_find_arg(args, "spid", "streetpassid", "0").c_str());

		smdh.settings.gameRatings[SMDH_RATING_CERO] = (u8) atoi(cmd_find_arg(args, "cer", "cero", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_ESRB] = (u8) atoi(cmd_find_arg(args, "er", "esrb", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_USK] = (u8) atoi(cmd_find_arg(args, "ur", "usk", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_PEGI_GEN] = (u8) atoi(cmd_find_arg(args, "pgr", "pegigen", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_PEGI_PTR] = (u8) atoi(cmd_find_arg(args, "ppr", "pegiptr", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_PEGI_BBFC] = (u8) atoi(cmd_find_arg(args, "pbr", "pegibbfc", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_COB] = (u8) atoi(cmd_find_arg(args, "cor", "cob", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_GRB] = (u8) atoi(cmd_find_arg(args, "gr", "grb", "0").c_str());
		smdh.settings.gameRatings[SMDH_RATING_CGSRR] = (u8) atoi(cmd_find_arg(args, "cgr", "cgsrr", "0").c_str());

		return cmd_make_smdh(smdh, icon, output);
	} else if(strcmp(command, "makecwav") == 0) {
		const fs::path input = cmd_find_arg(args, "i", "input", "");
		const fs::path output = cmd_find_arg(args, "o", "output", "");
		std::string loop = cmd_find_arg(args, "l", "loop", "false");
		u32 loopStartFrame = (u32) atoi(cmd_find_arg(args, "s", "loopstartframe", "0").c_str());
		u32 loopEndFrame = (u32) atoi(cmd_find_arg(args, "e", "loopendframe", "0").c_str());
		if(input.empty() || output.empty()) {
			cmd_missing_args(command);
			return -1;
		}

		std::transform(loop.begin(), loop.end(), loop.begin(), (int (*)(int)) std::tolower);
		if(loop != "false" && loop != "true") {
			cmd_invalid_arg("loop", command);
			return -1;
		}

		return cmd_make_cwav(input, output, loop == "true", loopStartFrame, loopEndFrame);
	} else if(strcmp(command, "lz11") == 0) {
		const fs::path input = cmd_find_arg(args, "i", "input", "");
		const fs::path output = cmd_find_arg(args, "o", "output", "");
		if(input.empty() || output.empty()) {
			cmd_missing_args(command);
			return -1;
		}

		return cmd_lz11(input, output);
	}

	cmd_invalid_command(command);
	return -1;
}
