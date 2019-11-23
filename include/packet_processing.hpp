// File: packet_processing.hpp
// Purpose: Packet processing pipeline
// Author: jeferson.silva@polymtl.ca

// STL libs
#include <cstdint>
#include <array>
#include <tuple>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <typeinfo>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>


#ifndef __PKT_PROCESSING__
#define __PKT_PROCESSING__

#include "pkt_common.hpp"
#include "static_hash_map.hpp"
#include "lookup_table.hpp"
#include "stats.hpp"

using namespace boost::accumulators;

template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class PacketProcessing {

    public:
        // Constants
        static constexpr auto LOOKUP_MEM_SIZE = Lookup_Size;
        using lookup_table_t = LookupTable<Lookup_Size, Lookup_Value>;

        void process_packet (const bool run_forever, const std::size_t read_step, const CacheType cache_type)
        {

            while (true) {
                // Wait until a message is pushed to the queue
                auto [timeout, tmp_discrete] = in_comm_pkt_.pull_message(packet_timestamp_, read_step);
                discrete_ts = tmp_discrete  ;

                // Exit in case of timeout
                if (timeout) {
                    print_status();
                    break;
                }

                // Extract five tuple and packet size
                tuple_size_pair_ = create_five_tuple_from_packet(packet_timestamp_.first);
                num_packets_++;
                num_bytes_+=tuple_size_pair_.second;
                debug(
                std::cout << " Thread ID " << std::this_thread::get_id() << " extracted " << num_packets_ << " five tuples\n";
                std::cout << tuple_size_pair_.first << '\n';
                )
                // Look table iterator: key + value
                auto lookup_result = lookup_table_.find(tuple_size_pair_.first);
                auto match = lookup_result != lookup_table_.end();

                // Is not held in the cache?
                if (!match) {
                    punt_pkt_to_next_lvl(out_comm_pkt_);
                    digest_pkt_to_ctrl(digest_cpu_, cache_type);
                } else {
                    matched_packets_++;
                    matched_bytes_+=tuple_size_pair_.second;
                    debug(std::cout << "Matched " << tuple_size_pair_.first << "Cache: " << ((lookup_table_.is_full()) ? "full" : "not full") << '\n';)
                }

                const auto hit_ratio =  matched_packets_/double(num_packets_);
                const auto weighted_hit_ratio =  matched_bytes_/double(num_bytes_);
                vec_hit_ratio_(hit_ratio);
                vec_weghted_hit_ratio_(weighted_hit_ratio);

                debug(print_status();)

                // Update cache defined in the derived
                update_cache_stats(match, cache_type);


                if (num_packets_%100000 == 0)
                {
                    print_status();
                }
                if (!run_forever) {
                    debug(print_status();)
                    break;
                }
                
                if constexpr (Sleep_Time) {
                    std::this_thread::sleep_for(nano_second_t(Sleep_Time));
                }
            }
        }

        void print_status()
        {
            const std::string full = ((lookup_table_.is_full()) ? "full" : "not full");
            std::cout << "Total packets: " << num_packets_ << '\n';
            std::cout << "Total matches: " << matched_packets_ << '\n';
            std::cout << "Normalized Hit Ratio: " << matched_packets_/double(num_packets_) << '\n';
            std::cout << "AVG Hit Ratio: " << mean(vec_hit_ratio_) << ", cache " << full<<'\n';
            std::cout << "Variance Hit Ratio: " << variance(vec_hit_ratio_) << ", cache " << full<<'\n';
            std::cout << "Weighted Normalized Hit Ratio: " << matched_bytes_/double(num_bytes_) << '\n';
            std::cout << "Weighted AVG Hit Ratio: " << mean(vec_weghted_hit_ratio_) << ", cache " << full<<'\n';
            std::cout << "Weighted Variance Hit Ratio: " << variance(vec_weghted_hit_ratio_) << ", cache " << full<<'\n';
        }

        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt)=0;

        // TODO: Send to policer in case of a cache miss
        void digest_pkt_to_ctrl (inter_thread_digest_cpu& digest_pkt, CacheType cache_type) {
            if(cache_type == CacheType::OPT){ 
                digest_pkt.push_message({tuple_size_pair_.first,discrete_ts});

            }
            else{
                digest_pkt.push_message(tuple_size_pair_);
            }
        }

        virtual void update_cache_stats(const bool match, const CacheType cache_type) =0;

        auto& lookup_table () { return lookup_table_; }

        auto& stats_table () { return stats_table_; }

        PacketProcessing(inter_thread_comm_t& in_comm_pkt,
                            inter_thread_comm_t& out_comm_pkt,
                            inter_thread_digest_cpu& digest_cpu) :
                            in_comm_pkt_(in_comm_pkt),
                            out_comm_pkt_(out_comm_pkt),
                            digest_cpu_(digest_cpu)
        {}

    protected:
        LookupTable<Lookup_Size, Lookup_Value> lookup_table_;
        inter_thread_comm_t& in_comm_pkt_;
        inter_thread_comm_t& out_comm_pkt_;
        inter_thread_digest_cpu& digest_cpu_;

        // Add stats container
        Cache_Stats stats_table_;
        // Add policy - for stats support
        packet_timestamp_pair_t packet_timestamp_;
        std::size_t num_packets_ = 0;
        std::size_t num_bytes_ = 0;
        std::size_t matched_packets_ = 0;
        std::size_t matched_bytes_ = 0;
        accumulator_set<double, features<tag::mean, tag::variance>> vec_hit_ratio_;
        accumulator_set<double, features<tag::mean, tag::variance>> vec_weghted_hit_ratio_;
        std::size_t discrete_ts = 0;
        tuple_pkt_size_pair_t tuple_size_pair_;

}; // PacketProcessing


template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class CacheL1PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time> {

    public:
        using pkt_proc_base_t = PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time>;

        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt) override {
            punted_pkt.push_message(this->packet_timestamp_);
        }

        virtual void update_cache_stats(const bool match, const CacheType cache_type) override {
            if (match) {
                if (cache_type == CacheType::LRU ||cache_type == CacheType::OPT) {
                    this->stats_table_.update_stats(this->tuple_size_pair_.first, this->discrete_ts);
                } else if (cache_type == CacheType::LFU) {
                    this->stats_table_.update_stats(this->tuple_size_pair_.first, this->tuple_size_pair_.second);
                } else {
                }
            }
        }

        // CTOR calls the base CTOR
        CacheL1PacketProcessing(inter_thread_comm_t& in_comm_pkt,
                                    inter_thread_comm_t& out_comm_pkt,
                                    inter_thread_digest_cpu& digest_cpu) :
                                    pkt_proc_base_t(in_comm_pkt, out_comm_pkt, digest_cpu)
        {}

};

template<size_t Lookup_Size, typename Lookup_Value, typename Cache_Stats, size_t Sleep_Time = 0>
class CacheL2PacketProcessing final : public PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time> {

    public:
        using pkt_proc_base_t = PacketProcessing <Lookup_Size, Lookup_Value, Cache_Stats, Sleep_Time>;

        virtual void punt_pkt_to_next_lvl (inter_thread_comm_t& punted_pkt) override {}
        virtual void update_cache_stats(const bool match, const CacheType cache_type) override {}

        // CTOR calls the base CTOR
        CacheL2PacketProcessing(inter_thread_comm_t& in_comm_pkt,
                                    inter_thread_comm_t& out_comm_pkt,
                                    inter_thread_digest_cpu& digest_cpu) :
                                    pkt_proc_base_t(in_comm_pkt, out_comm_pkt, digest_cpu)
        {}

};

#endif // __PKT_PROCESSING__
