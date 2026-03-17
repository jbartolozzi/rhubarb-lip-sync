#include "animationRules.h"
#include <boost/algorithm/clamp.hpp>
#include "shapeShorthands.h"
#include "tools/array.h"
#include "time/ContinuousTimeline.h"

using std::chrono::duration_cast;
using boost::algorithm::clamp;
using boost::optional;
using std::array;
using std::pair;
using std::map;

constexpr size_t shapeValueCount = static_cast<size_t>(Shape::EndSentinel);

Shape getBasicShape(Shape shape) {
	//                  A  B  C  D  E  F  G  H  I  J  K  L  X
	static constexpr array<Shape, shapeValueCount> basicShapes =
		make_array(A, B, C, D, E, F, A, C, B, B, F, E, A);
	return basicShapes[static_cast<size_t>(shape)];
}

Shape relax(Shape shape) {
	//                  A  B  C  D  E  F  G  H  I  J  K  L  X
	static constexpr array<Shape, shapeValueCount> relaxedShapes =
		make_array(A, B, B, C, C, B, X, B, B, X, B, B, X);
	return relaxedShapes[static_cast<size_t>(shape)];
}

Shape getClosestShape(Shape reference, ShapeSet shapes) {
	if (shapes.empty()) {
		throw std::invalid_argument("Cannot select from empty set of shapes.");
	}

	// A matrix that for each shape contains all shapes in ascending order of effort required to
	// move to them
	constexpr static array<array<Shape, shapeValueCount>, shapeValueCount> effortMatrix = make_array(
		/* A */ make_array(A, X, G, J, B, I, C, H, L, E, D, K, F),
		/* B */ make_array(B, I, G, J, A, X, C, H, L, E, D, K, F),
		/* C */ make_array(C, H, B, I, G, J, D, A, X, L, E, K, F),
		/* D */ make_array(D, C, H, B, I, G, J, A, X, L, E, K, F),
		/* E */ make_array(E, L, C, H, K, B, I, G, J, A, X, D, F),
		/* F */ make_array(F, K, E, L, B, I, G, J, A, X, C, H, D),
		/* G */ make_array(G, J, A, B, I, C, H, X, L, E, D, K, F),
		/* H */ make_array(H, C, B, I, G, J, D, A, X, L, E, K, F),
		/* I */ make_array(I, B, G, J, C, H, A, X, L, E, D, K, F),
		/* J */ make_array(J, G, A, X, B, I, C, H, L, E, D, K, F),
		/* K */ make_array(K, F, E, L, B, I, G, J, C, H, A, X, D),
		/* L */ make_array(L, E, K, C, H, B, I, F, G, J, A, X, D),
		/* X */ make_array(X, A, G, J, B, I, C, H, L, E, D, K, F)
	);

	auto& closestShapes = effortMatrix.at(static_cast<size_t>(reference));
	for (Shape closestShape : closestShapes) {
		if (shapes.find(closestShape) != shapes.end()) {
			return closestShape;
		}
	}

	throw std::invalid_argument("Unable to find closest shape.");
}

optional<pair<Shape, TweenTiming>> getTween(Shape first, Shape second) {
	// Note that most of the following rules work in one direction only.
	// That's because in animation, the mouth should usually "pop" open without inbetweens,
	// then close slowly.
	static const map<pair<Shape, Shape>, pair<Shape, TweenTiming>> lookup {
		{ { D, A }, { C, TweenTiming::Early } },
		{ { D, B }, { C, TweenTiming::Centered } },
		{ { D, G }, { C, TweenTiming::Early } },
		{ { D, I }, { C, TweenTiming::Centered } },
		{ { D, J }, { C, TweenTiming::Early } },
		{ { D, X }, { C, TweenTiming::Late } },
		{ { C, F }, { E, TweenTiming::Centered } }, { { F, C }, { E, TweenTiming::Centered } },
		{ { C, K }, { E, TweenTiming::Centered } }, { { K, C }, { E, TweenTiming::Centered } },
		{ { D, F }, { E, TweenTiming::Centered } },
		{ { D, K }, { E, TweenTiming::Centered } },
		{ { H, F }, { E, TweenTiming::Late } }, { { F, H }, { E, TweenTiming::Early } },
		{ { H, K }, { E, TweenTiming::Late } }, { { K, H }, { E, TweenTiming::Early } }
	};
	const auto it = lookup.find({ first, second });
	return it != lookup.end() ? it->second : optional<pair<Shape, TweenTiming>>();
}

Timeline<ShapeSet> getShapeSets(Phone phone, centiseconds duration, centiseconds previousDuration) {
	// Returns a timeline with a single shape set
	const auto single = [duration](ShapeSet value) {
		return Timeline<ShapeSet> { { 0_cs, duration, value } };
	};

	// Returns a timeline with two shape sets, timed as a diphthong
	const auto diphthong = [duration](ShapeSet first, ShapeSet second) {
		const centiseconds firstDuration = duration_cast<centiseconds>(duration * 0.6);
		return Timeline<ShapeSet> {
			{ 0_cs, firstDuration, first },
			{ firstDuration, duration, second }
		};
	};

	// Returns a timeline with two shape sets, timed as a plosive
	const auto plosive = [duration, previousDuration](ShapeSet first, ShapeSet second) {
		const centiseconds minOcclusionDuration = 4_cs;
		const centiseconds maxOcclusionDuration = 12_cs;
		const centiseconds occlusionDuration =
			clamp(previousDuration / 2, minOcclusionDuration, maxOcclusionDuration);
		return Timeline<ShapeSet> {
			{ -occlusionDuration, 0_cs, first },
			{ 0_cs, duration, second }
		};
	};

	// Returns the result of `getShapeSets` when called with identical arguments
	// except for a different phone.
	const auto like = [duration, previousDuration](Phone referencePhone) {
		return getShapeSets(referencePhone, duration, previousDuration);
	};

	static const ShapeSet any { A, B, C, D, E, F, G, H, I, J, K, L, X };
	static const ShapeSet anyOpen { B, C, D, E, F, G, H, I, J, K, L };

	// Note:
	// The shapes {A, B, G, X} are very similar. You should avoid regular shape sets containing more
	// than one of these shapes.
	// Otherwise, the resulting shape may be more or less random and might not be a good fit.
	// As an exception, a very flexible rule may contain *all* these shapes.

	switch (phone) {
		case Phone::AO: return single({ E });
		case Phone::AA: return single({ D });
		case Phone::IY: return single({ I });
		case Phone::UW: return single({ F });
		case Phone::EH: return single({ C });
		case Phone::IH: return single({ B });
		case Phone::UH: return single({ F });
		case Phone::AH: return duration < 20_cs ? single({ C }) : single({ D });
		case Phone::Schwa: return single({ B, C });
		case Phone::AE: return single({ C });
		case Phone::EY: return diphthong({ C }, { I });
		case Phone::AY: return duration < 20_cs ? diphthong({ C }, { I }) : diphthong({ D }, { I });
		case Phone::OW: return diphthong({ E }, { F });
		case Phone::AW: return duration < 30_cs ? diphthong({ C }, { E }) : diphthong({ D }, { E });
		case Phone::OY: return diphthong({ E }, { I });
		case Phone::ER: return duration < 7_cs ? like(Phone::Schwa) : single({ E });

		case Phone::P:
		case Phone::B: return plosive({ A }, any);
		case Phone::T:
		case Phone::D: return plosive({ B, F }, anyOpen);
		case Phone::K:
		case Phone::G: return plosive({ B, C, E, F, H }, anyOpen);
		case Phone::CH:
		case Phone::JH: return single({ K });
		case Phone::F:
		case Phone::V: return single({ G });
		case Phone::TH:
		case Phone::DH: return single({ J });
		case Phone::S:
		case Phone::Z:
		case Phone::SH:
		case Phone::ZH: return single({ K });
		case Phone::HH: return single(any); // think "m-hm"
		case Phone::M: return single({ A });
		case Phone::N: return single({ B, C, F, H });
		case Phone::NG: return single({ B, C, E, F });
		case Phone::L: return duration < 20_cs ? single({ B, E, F, H }) : single({ H });
		case Phone::R: return single({ L });
		case Phone::Y: return single({ B, C, F });
		case Phone::W: return single({ F });

		case Phone::Breath:
		case Phone::Cough:
		case Phone::Smack: return single({ C });
		case Phone::Noise: return single({ B });

		default: throw std::invalid_argument("Unexpected phone.");
	}
}
