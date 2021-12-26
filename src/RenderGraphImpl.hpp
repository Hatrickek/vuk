#pragma once

#include <vuk/ShortAlloc.hpp>
#include <robin_hood.h>
#include "RenderGraphUtil.hpp"

namespace vuk {
	struct CommandBundle {
		uint32_t pass_start;
		uint32_t pass_end;
	};

#define INIT(x) x(decltype(x)::allocator_type(*arena_))
	struct RGImpl {
		std::unique_ptr<arena> arena_;
		std::vector<PassInfo, short_alloc<PassInfo, 64>> passes;

		robin_hood::unordered_flat_map<Name, Name> aliases;
		robin_hood::unordered_flat_set<Name> poisoned_names;

		robin_hood::unordered_flat_map<Name, std::vector<UseRef, short_alloc<UseRef, 64>>> use_chains;

		std::vector<RenderPassInfo, short_alloc<RenderPassInfo, 64>> rpis;
		size_t num_graphics_rpis = 0;
		size_t num_transfer_rpis = 0;

		std::vector<std::pair<Name, Future<Buffer>*>> to_manage;

		std::vector<CommandBundle> command_bundles;

		robin_hood::unordered_flat_map<Name, AttachmentRPInfo> bound_attachments;
		robin_hood::unordered_flat_map<Name, BufferInfo> bound_buffers;

		RGImpl() : arena_(new arena(1024 * 128)), INIT(passes), INIT(rpis) {
		}

		Name resolve_name(Name in) {
			auto it = aliases.find(in);
			if (it == aliases.end())
				return in;
			else
				return resolve_name(it->second);
		};
	};
#undef INIT

	template<class T, class A, class F>
	T* contains_if(std::vector<T, A>& v, F&& f) {
		auto it = std::find_if(v.begin(), v.end(), f);
		if (it != v.end()) return &(*it);
		else return nullptr;
	}

	template<class T, class A, class F>
	T const* contains_if(const std::vector<T, A>& v, F&& f) {
		auto it = std::find_if(v.begin(), v.end(), f);
		if (it != v.end()) return &(*it);
		else return nullptr;
	}

	template<class T, class A>
	T const* contains(const std::vector<T, A>& v, const T& f) {
		auto it = std::find(v.begin(), v.end(), f);
		if (it != v.end()) return &(*it);
		else return nullptr;
	}

	template <typename Iterator, typename Compare>
	void topological_sort(Iterator begin, Iterator end, Compare cmp) {
		while (begin != end) {
			auto const new_begin = std::partition(begin, end, [&](auto const& a) {
				return std::none_of(begin, end, [&](auto const& b) { return cmp(b, a); });
				});
			assert(new_begin != begin && "not a partial ordering");
			begin = new_begin;
		}
	}
};