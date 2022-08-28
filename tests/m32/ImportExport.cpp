// Project AIDA
// Created by Long Duong on 7/4/22.
// Purpose: 
//
#include <catch2/catch.hpp>
#include <boost/filesystem.hpp>
#include "MxlImport.h"
#include "MxlExport.h"
#include "Score.h"
#include "Part.h"
#include "utility"
#include "types.h"
#include "show.h"
#include <libxml2/libxml/catalog.h>
#include <libxml2/libxml/parser.h>
#include "tinynote/PartParser.h"
#include "tinynote/MeasureParser.h"
#include "tinynote/TupletParser.h"
#include <fstream>
#include <iostream>

#include <cereal/archives/xml.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>

using namespace m32;
using gnote_t = m32::GNote<m32::Offset, m32::Duration>;
using part_t = m32::Part<gnote_t>;

TEST_CASE("import") {
    boost::filesystem::path mxl_path(TESTS_DIR);
    mxl_path /= "m32/mxl";
    SECTION("test 1") {
        auto p = MxlImporter<>();
        p.parse_file((mxl_path / "longduongs.musicxml").c_str());

        auto mpart1 = tinynote::parse_measured_part(R"(
MeasuredPart[time=4/4, clef=treble, key=4] {
    Mea [time=4/4, clef=treble, key=0] {
        B3:0.75[color=#B40003],
        E4:0.25[color=#1A14A9],
        F#4:0.25,
        G4:0.5,
        E4:0.5[lyrics=["long", "duong"]],
        A4:0.5,
        G4:0.5,
        F4:0.25,
        E4:0.25,
        F4:0.25[tie_info=TieStart]
    },
    Mea {
        F4:0.25[tie_info=TieEnd],
        D4:0.5,
        C5:0.5,
        B4:0.5,
        rest:0.25,
        Tup@4!6[ F5:1/3, E5:1/3, D5:1/3, C5:1/3, B4:1/3, A4:1/3 ]:2
    },
    Mea {
        Tup@4!5[
            Ch[F4, A4]:0.8,
            rest:0.8,
            Ch[B4, C5, E5]:0.8,
            Ch[G4, D5]:0.8,
            B4:0.4,
            Ch[G4, B4, D5]:0.4
        ]:4
    },
    Mea { rest:4 }, Mea{ rest:4 }
}
)");
        using mpart = decltype(mpart1);

        auto firstparteq = p.mscore.measured_parts[0]._eq<
                mpart::ClefSignCmp,
                mpart::KeySigCmp,
                mpart::TimeSigCmp,

                mpart::MeasureT::MeaNumberCmp,
                mpart::MeasureT::MeaClefSignCmp,
                mpart::MeasureT::MeaKeySigCmp,
                mpart::MeasureT::MeaTimeSig,

                gnote_t::TupletT::NormalCmp,
                gnote_t::TupletT::ActualCmp,

                gnote_t::SimpleNoteT::StartCmp,
                gnote_t::SimpleNoteT::EndCmp,
                gnote_t::SimpleNoteT::LengthCmp,

                gnote_t::TupletT::StartCmp,
                gnote_t::TupletT::EndCmp,
                gnote_t::TupletT::LengthCmp,

                gnote_t::SimpleNoteT::PitchCmpExact,
                gnote_t::SimpleNoteT::LyricCmp,
                gnote_t::SimpleNoteT::TieInfoCmp,
                // gnote_t::SimpleNoteT::DynamicCmp,
                gnote_t::SimpleNoteT::ColorCmp
                >(mpart1);
        REQUIRE(firstparteq);

        auto mpart2 = tinynote::parse_measured_part(R"(
MeasuredPart[time=4/4, clef=bass, key=0] {
    Mea [time=4/4] {
        Tup@4!5[
            Ch[F2, A2, C3]:0.6,
            Ch[B2, D3, F3]:0.3,
            Ch[D3, G3, C#4]:0.1,
            Ch[D3, G3, C#4]:0.2,
            Ch[F3, A3, C4]:0.4,
            Ch[B3, E4, G4]:0.4
        ]:2,
        Tup@2!3[
            Ch[B2, D3, G3]:4/3,
            Ch[F2, B2, D3]:2/3[tie_info=TieStart]
        ]:2
    },
    Mea [start=4, length=4] {
        Ch[F2, B2, D3]:0.25[tie_info=TieEnd],
        rest:0.25,
        rest:0.5,
        Tup@4!5[
            D2:0.2,
            rest:0.2,
            F3:0.2,
            C3:0.2,
            rest:0.2
        ]:1,
        Tup@2!3[
            C3:1,
            rest:1/6,
            B2:0.5,
            D3:1/3
        ]:2
    },
    Mea [start=8, length=4] {
        Ch[G2, C3, E3]:4
    },
    Mea [start=12, length=4] {
        Ch[E2, B2, E3]:3,
        Ch[A2, C3, G3]:0.5,
        Ch[B2, G3, C4]:3/8,
        rest:1/8
     },
    Mea [start=16, length=4] { rest:4 }
}
)");
        // std::cout << p.mscore.measured_parts[1] << mpart2 << std::endl;
        auto secondparteq = p.mscore.measured_parts[1]._eq<
                // mpart::ClefSignCmp,
                mpart::KeySigCmp,
                mpart::TimeSigCmp,

                mpart::MeasureT::MeaNumberCmp,
                mpart::MeasureT::MeaClefSignCmp,
                mpart::MeasureT::MeaKeySigCmp,
                mpart::MeasureT::MeaTimeSig,

                 gnote_t::TupletT::NormalCmp,
                 gnote_t::TupletT::ActualCmp,

                 gnote_t::SimpleNoteT::StartCmp,
                gnote_t::SimpleNoteT::EndCmp,
                gnote_t::SimpleNoteT::LengthCmp,

                gnote_t::TupletT::StartCmp,
                gnote_t::TupletT::EndCmp,
                gnote_t::TupletT::LengthCmp,

                gnote_t::SimpleNoteT::PitchCmpExact,
                 gnote_t::SimpleNoteT::LyricCmp,
                gnote_t::SimpleNoteT::TieInfoCmp,
                 //gnote_t::SimpleNoteT::DynamicCmp,
                 gnote_t::SimpleNoteT::ColorCmp
        >(mpart2);
        REQUIRE(secondparteq);

    }
    SECTION("test2") {
        auto p = MxlImporter<>();
        REQUIRE_NOTHROW(p.parse_file((mxl_path / "eoutmax.musicxml").c_str()));
        p.reset();
        REQUIRE_NOTHROW(p.parse_file((mxl_path / "example1.musicxml").c_str()));
    }
}

TEST_CASE("export") {
    using mscore_t = m32::MeasuredScore<m32::Offset, m32::Duration>;

    SECTION("simple-note") {
        auto exp = MxlExporter<mscore_t>();
        xml_document score_doc;
        auto* measure_tag = score_doc.allocate_node(node_type::node_element, "measure");
        score_doc.append_node(measure_tag);
        auto snote = tinynote::parse_simple_note("Ch[C5, D2]:1/2");
        assert(snote.tie_info == TieInfo::TieNeither);
        exp.add_simple_note(measure_tag, snote, 10);
        // print(std::cout, score_doc);

        xml_document man_doc;
        man_doc.parse<0>(man_doc.allocate_string(R"(
<measure>
	<note>
		<pitch>
			<step>C</step>
			<octave>5</octave>
		</pitch>
		<duration>5</duration>
		<type>eighth</type>
	</note>
	<note>
		<chord/>
		<pitch>
			<step>D</step>
			<octave>2</octave>
		</pitch>
		<duration>5</duration>
		<type>eighth</type>
	</note>
</measure>
)"));

        REQUIRE(compare_xml_node(man_doc.first_node(), score_doc.first_node()));
    }

    SECTION("simple-note-with durations requiring ties") {
        auto exp = MxlExporter<mscore_t>();
        xml_document score_doc;
        auto* measure_tag = score_doc.allocate_node(node_type::node_element, "measure");
        score_doc.append_node(measure_tag);
        auto snote = tinynote::parse_simple_note("Ch[C5, D2]:275/100");
        assert(snote.tie_info == TieInfo::TieNeither);
        exp.add_simple_note(measure_tag, snote, 100);
        // print(std::cout, score_doc);

        xml_document man_doc;
        man_doc.parse<0>(man_doc.allocate_string(R"(
<measure>
	<note>
		<pitch>
			<step>C</step>
			<octave>5</octave>
		</pitch>
		<duration>200</duration>
		<tie type="start"/>
		<type>half</type>
		<notations>
			<tied type="start"/>
		</notations>
	</note>
	<note>
		<chord/>
		<pitch>
			<step>D</step>
			<octave>2</octave>
		</pitch>
		<duration>200</duration>
		<tie type="start"/>
		<type>half</type>
		<notations>
			<tied type="start"/>
		</notations>
	</note>
	<note>
		<pitch>
			<step>C</step>
			<octave>5</octave>
		</pitch>
		<duration>75</duration>
		<tie type="stop"/>
		<type>eighth</type>
		<dot/>
		<notations>
			<tied type="stop"/>
		</notations>
	</note>
	<note>
		<chord/>
		<pitch>
			<step>D</step>
			<octave>2</octave>
		</pitch>
		<duration>75</duration>
		<tie type="stop"/>
		<type>eighth</type>
		<dot/>
		<notations>
			<tied type="stop"/>
		</notations>
	</note>
</measure>
)"));

        REQUIRE(compare_xml_node(man_doc.first_node(), score_doc.first_node()));
    }

    SECTION("test 1") {
        auto mpart = tinynote::parse_measured_part(R"(
MeasuredPart[time=4/4, clef=treble, key=4] {
    Mea [time=4/4, clef=treble, key=4] {
        C#4:2,
        Tup@2[Ch[C4, G4]:1, Ch[G5, B3]:1, Ch[A3, C3]:1, Cb6:1]:2
    },
    Mea [start=4, length=4] {
        C#4:2,
        Ch[C4, G5, B6]:2[tie_info=TieStart+TieEnd]
    }
}
)");
//        std::cout << mpart << std::endl;
//        using mscore_t = m32::MeasuredScore<m32::Offset, m32::Duration>;
//        auto score = mscore_t("tinynote");
//        score.measured_parts.push_back(mpart);
//        auto exp = MxlExporter<mscore_t>();
//
//        // std::ofstream fout("tinynote.musicxml");
//        // exp.xml_to_stream(fout, score);
    }
    SECTION("test2") {
        boost::filesystem::path mxl_path(TESTS_DIR);
        mxl_path /= "m32/mxl";
        auto p = MxlImporter<>();
        p.parse_file((mxl_path / "longduongs.musicxml").c_str());
        // std::cout << p.mscore << std::endl;

        auto exp = MxlExporter<mscore_t, false>();
        auto& sc_doc = exp.build_xml_doc(p.mscore);
        // print(std::cout, sc_doc);

        xml_document man_doc;
        man_doc.parse<0>(man_doc.allocate_string(R"(
<score-partwise version="3.1">
	<work>
		<work-title>template</work-title>
	</work>
	<identification>
		<creator type="composer">LDAida</creator>
		<encoding>
			<software>Aida</software>
		</encoding>
	</identification>
	<part-list>
		<score-part id="P1">
			<part-name>Soprano</part-name>
		</score-part>
		<score-part id="P2">
			<part-name>Bass</part-name>
		</score-part>
		<score-part id="P3">
			<part-name>Bass</part-name>
		</score-part>
	</part-list>
	<part id="P1">
		<measure number="1">
			<attributes>
				<divisions>60</divisions>
				<key>
					<fifths>0</fifths>
				</key>
				<time>
					<beats>4</beats>
					<beat-type>4</beat-type>
				</time>
				<clef>
					<sign>G</sign>
				</clef>
			</attributes>
			<note color="#B40003">
				<pitch>
					<step>B</step>
					<octave>3</octave>
				</pitch>
				<duration>45</duration>
				<type>eighth</type>
				<dot/>
			</note>
			<note color="#1A14A9">
				<pitch>
					<step>E</step>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<type>16th</type>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<alter>1</alter>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<type>16th</type>
			</note>
			<note>
				<pitch>
					<step>G</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>E</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
				<lyric number="0">
					<text>long</text>
				</lyric>
				<lyric number="2">
					<text>duong</text>
				</lyric>
			</note>
			<note>
				<pitch>
					<step>A</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>G</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<type>16th</type>
			</note>
			<note>
				<pitch>
					<step>E</step>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<type>16th</type>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<tie type="start"/>
				<type>16th</type>
				<notations>
					<tied type="start"/>
				</notations>
			</note>
		</measure>
		<measure number="2">
			<note>
				<pitch>
					<step>F</step>
					<octave>4</octave>
				</pitch>
				<duration>15</duration>
				<tie type="stop"/>
				<type>16th</type>
				<notations>
					<tied type="stop"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>5</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>4</octave>
				</pitch>
				<duration>30</duration>
				<type>eighth</type>
			</note>
			<note>
				<rest/>
				<duration>15</duration>
				<type>16th</type>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>5</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>E</step>
					<octave>5</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>5</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>5</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>4</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>A</step>
					<octave>4</octave>
				</pitch>
				<duration>20</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>6</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
		</measure>
		<measure number="3">
			<note>
				<pitch>
					<step>F</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>A</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<rest/>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>5</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>E</step>
					<octave>5</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>G</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>5</octave>
				</pitch>
				<duration>48</duration>
				<type>quarter</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>4</octave>
				</pitch>
				<duration>24</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>G</step>
					<octave>4</octave>
				</pitch>
				<duration>24</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>B</step>
					<octave>4</octave>
				</pitch>
				<duration>24</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>5</octave>
				</pitch>
				<duration>24</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
		</measure>
		<measure number="4">
			<note>
				<rest/>
				<duration>240</duration>
				<type>whole</type>
			</note>
		</measure>
		<measure number="5">
			<note>
				<rest/>
				<duration>240</duration>
				<type>whole</type>
			</note>
		</measure>
	</part>
	<part id="P2">
		<measure number="1">
			<attributes>
				<divisions>120</divisions>
				<key>
					<fifths>0</fifths>
				</key>
				<time>
					<beats>4</beats>
					<beat-type>4</beat-type>
				</time>
				<clef>
					<sign>F</sign>
				</clef>
			</attributes>
			<note>
				<pitch>
					<step>F</step>
					<octave>2</octave>
				</pitch>
				<duration>72</duration>
				<type>eighth</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>A</step>
					<octave>2</octave>
				</pitch>
				<duration>72</duration>
				<type>eighth</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>3</octave>
				</pitch>
				<duration>72</duration>
				<type>eighth</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>36</duration>
				<type>16th</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>36</duration>
				<type>16th</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>F</step>
					<octave>3</octave>
				</pitch>
				<duration>36</duration>
				<type>16th</type>
				<dot/>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>12</duration>
				<type>32nd</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>3</octave>
				</pitch>
				<duration>12</duration>
				<type>32nd</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<alter>1</alter>
					<octave>4</octave>
				</pitch>
				<duration>12</duration>
				<type>32nd</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>3</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<alter>1</alter>
					<octave>4</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>3</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>A</step>
					<octave>3</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>3</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>E</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>4</octave>
				</pitch>
				<duration>48</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>160</duration>
				<type>half</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>160</duration>
				<type>half</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>3</octave>
				</pitch>
				<duration>160</duration>
				<type>half</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>2</octave>
				</pitch>
				<duration>80</duration>
				<tie type="start"/>
				<type>quarter</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
					<tied type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>80</duration>
				<tie type="start"/>
				<type>quarter</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
					<tied type="start"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>80</duration>
				<tie type="start"/>
				<type>quarter</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
					<tied type="start"/>
				</notations>
			</note>
		</measure>
		<measure number="2">
			<note>
				<pitch>
					<step>F</step>
					<octave>2</octave>
				</pitch>
				<duration>30</duration>
				<tie type="stop"/>
				<type>16th</type>
				<notations>
					<tied type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>30</duration>
				<tie type="stop"/>
				<type>16th</type>
				<notations>
					<tied type="stop"/>
				</notations>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>30</duration>
				<tie type="stop"/>
				<type>16th</type>
				<notations>
					<tied type="stop"/>
				</notations>
			</note>
			<note>
				<rest/>
				<duration>30</duration>
				<type>16th</type>
			</note>
			<note>
				<rest/>
				<duration>60</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>2</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<rest/>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>F</step>
					<octave>3</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>3</octave>
				</pitch>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
			</note>
			<note>
				<rest/>
				<duration>24</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>3</octave>
				</pitch>
				<duration>120</duration>
				<type>quarter</type>
				<dot/>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<rest/>
				<duration>20</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>60</duration>
				<type>eighth</type>
				<dot/>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>D</step>
					<octave>3</octave>
				</pitch>
				<duration>40</duration>
				<type>eighth</type>
				<time-modification>
					<actual-notes>3</actual-notes>
					<normal-notes>2</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
		</measure>
		<measure number="3">
			<note>
				<pitch>
					<step>G</step>
					<octave>2</octave>
				</pitch>
				<duration>480</duration>
				<type>whole</type>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>3</octave>
				</pitch>
				<duration>480</duration>
				<type>whole</type>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>E</step>
					<octave>3</octave>
				</pitch>
				<duration>480</duration>
				<type>whole</type>
			</note>
		</measure>
		<measure number="4">
			<note>
				<pitch>
					<step>E</step>
					<octave>2</octave>
				</pitch>
				<duration>360</duration>
				<type>half</type>
				<dot/>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>360</duration>
				<type>half</type>
				<dot/>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>E</step>
					<octave>3</octave>
				</pitch>
				<duration>360</duration>
				<type>half</type>
				<dot/>
			</note>
			<note>
				<pitch>
					<step>A</step>
					<octave>2</octave>
				</pitch>
				<duration>60</duration>
				<type>eighth</type>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>3</octave>
				</pitch>
				<duration>60</duration>
				<type>eighth</type>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>3</octave>
				</pitch>
				<duration>60</duration>
				<type>eighth</type>
			</note>
			<note>
				<pitch>
					<step>B</step>
					<octave>2</octave>
				</pitch>
				<duration>45</duration>
				<type>16th</type>
				<dot/>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>G</step>
					<octave>3</octave>
				</pitch>
				<duration>45</duration>
				<type>16th</type>
				<dot/>
			</note>
			<note>
				<chord/>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>45</duration>
				<type>16th</type>
				<dot/>
			</note>
			<note>
				<rest/>
				<duration>15</duration>
				<type>32nd</type>
			</note>
		</measure>
		<measure number="5">
			<note>
				<rest/>
				<duration>480</duration>
				<type>whole</type>
			</note>
		</measure>
	</part>
	<part id="P3">
		<measure number="1">
			<attributes>
				<divisions>1</divisions>
				<key>
					<fifths>0</fifths>
				</key>
				<time>
					<beats>4</beats>
					<beat-type>4</beat-type>
				</time>
				<clef>
					<sign>F</sign>
				</clef>
			</attributes>
			<note color="#A29605">
				<rest/>
				<duration>1</duration>
				<type>quarter</type>
			</note>
			<note>
				<rest/>
				<duration>1</duration>
				<type>quarter</type>
			</note>
			<note>
				<rest/>
				<duration>2</duration>
				<type>half</type>
			</note>
		</measure>
		<measure number="2">
			<note>
				<rest/>
				<duration>4</duration>
				<type>whole</type>
			</note>
		</measure>
		<measure number="3">
			<note>
				<rest/>
				<duration>4</duration>
				<type>whole</type>
			</note>
		</measure>
		<measure number="4">
			<note>
				<rest/>
				<duration>4</duration>
				<type>whole</type>
			</note>
		</measure>
		<measure number="5">
			<note>
				<rest/>
				<duration>4</duration>
				<type>whole</type>
			</note>
		</measure>
	</part>
</score-partwise>
)"));

        // std::ofstream fout("tinynote.musicxml");
        // fout << sc_doc;

        REQUIRE(compare_xml_node(man_doc.first_node(), sc_doc.first_node()));
    }

    SECTION("tricky tuplet split") {
        auto tup = tinynote::parse_tuplet("Tup@4!5[C4:0.25, C4:0.25, C4:0.25, C4:0.25, C4:0.25]:1");
        tup.pack();
        auto pair = tup.split_at_offset(m32::Duration{1, 4});
        auto [left, right] = pair;

        auto msc = mscore_t("");
        auto mprt = std::remove_cvref_t<decltype(msc.measured_parts[0])>("", 0, 'G', {4, 4});
        mprt.append_gnote(left);
        mprt.append_gnote(right);

        xml_document out_doc;
        xml_node<>* out_node = out_doc.allocate_node(node_type::node_element, "score-partwise");
        out_doc.append_node(out_node);

        auto exp = m32::MxlExporter<mscore_t, false>();
        exp.add_part(out_node, "P1", mprt);
        //print(std::cout, out_doc);
        xml_document man_doc;
        man_doc.parse<0>(man_doc.allocate_string(R"(
<score-partwise>
	<part id="P1">
		<measure number="0">
			<attributes>
				<divisions>20</divisions>
				<key>
					<fifths>0</fifths>
				</key>
				<time>
					<beats>4</beats>
					<beat-type>4</beat-type>
				</time>
				<clef>
					<sign>G</sign>
				</clef>
			</attributes>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>4</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>1</duration>
				<tie type="start"/>
				<type>64th</type>
				<time-modification>
					<actual-notes>5</actual-notes>
					<normal-notes>4</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
					<tied type="start"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>3</duration>
				<tie type="stop"/>
				<type>32nd</type>
				<dot/>
				<time-modification>
					<actual-notes>15</actual-notes>
					<normal-notes>12</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="start"/>
					<tied type="stop"/>
				</notations>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>4</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>15</actual-notes>
					<normal-notes>12</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>4</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>15</actual-notes>
					<normal-notes>12</normal-notes>
				</time-modification>
			</note>
			<note>
				<pitch>
					<step>C</step>
					<octave>4</octave>
				</pitch>
				<duration>4</duration>
				<type>16th</type>
				<time-modification>
					<actual-notes>15</actual-notes>
					<normal-notes>12</normal-notes>
				</time-modification>
				<notations>
					<tuplet type="stop"/>
				</notations>
			</note>
		</measure>
	</part>
</score-partwise>
)"));
        // m32::show::show(mprt);
        REQUIRE(compare_xml_node(out_doc.first_node(), man_doc.first_node()));
    }
}


TEST_CASE("cereal-serialization") {
    SECTION("pitch") {
        std::stringstream ss;
        cereal::XMLOutputArchive archive(ss);
        m32::Pitch pitch = tinynote::parse_pitch("A#5");
        archive(pitch);
    }
    SECTION("simple-note") {
        std::stringstream ss;
        cereal::XMLOutputArchive archive(ss);
        auto note = tinynote::parse_simple_note("Ch[A#5, B4]:2/6");
        archive(note);
    }
    SECTION("whole score") {
        std::ofstream fout("binaryeoutmax.b", std::ios::binary);
        cereal::PortableBinaryOutputArchive archive(fout);

        boost::filesystem::path mxl_path(TESTS_DIR);
        mxl_path /= "m32/mxl";
        auto p = MxlImporter<>();
        p.parse_file((mxl_path / "eoutmax.musicxml").c_str());
        {
            archive(p.mscore);
            fout.flush();
        }

        std::ifstream fin("binaryeoutmax.b", std::ios::binary);
        std::streamsize size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        fin.read(buffer.data(), size);

        cereal::PortableBinaryInputArchive iarch(fin);

        MeasuredScore<m32::Offset, m32::Duration> in_mscore;
        iarch(in_mscore);
        REQUIRE(in_mscore.exact_eq(p.mscore));
    }

    SECTION("whole score -2") {
        std::ofstream fout("longduongs.b", std::ios::binary);
        cereal::PortableBinaryOutputArchive archive(fout);

        boost::filesystem::path mxl_path(TESTS_DIR);
        mxl_path /= "m32/mxl";
        auto p = MxlImporter<>();
        p.parse_file((mxl_path / "longduongs.musicxml").c_str());
        {
            archive(p.mscore);
            fout.flush();
        }

        std::ifstream fin("longduongs.b", std::ios::binary);
        std::streamsize size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        fin.read(buffer.data(), size);

        cereal::PortableBinaryInputArchive iarch(fin);

        MeasuredScore<m32::Offset, m32::Duration> in_mscore;
        iarch(in_mscore);
        REQUIRE(in_mscore.exact_eq(p.mscore));
    }
}