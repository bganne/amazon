#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cstdint>
#include <ctime>

#ifndef FR_BENOU_STATS_H_
#define FR_BENOU_STATS_H_

namespace fr_benou {

    /*
     * Functor for getting UNIX timestamp
     */
    struct GetTimestamp {
            /*
             * @timestamp_type: timestamp values type
             * @operator(): return UNIX timestamp
             */
            typedef std::uint64_t timestamp_type;
            timestamp_type operator() (void) const { return std::time(NULL); }
    };

    /*
     * Stats: store timestamped values upon a maximum lifetime
     * Support efficient elements insertion and percentile retrieval
     * It behaves like a STL container, but do not have the full semantic
     *
     * Template parameters:
     * @T: the value type
     * @TIMEOUT: max lifetime for values
     * @GETTIMESTAMP: Callable functor returning the current timestamp when called
     *
     */
    template <typename T=double, int TIMEOUT=60, typename GETTIMESTAMP=GetTimestamp> class Stats {
        /*
         * @timestamp_type: timestamp values type
         * @value_type: stored values type
         * @StatsPair: a std::pair<> containing (timestamp, value)
         * @size_type: a type large enough to count all stored elements
         */
        public:
            typedef typename GETTIMESTAMP::timestamp_type timestamp_type;
            typedef T value_type;
            typedef std::pair<timestamp_type, value_type> StatsPair;
            typedef typename std::vector<StatsPair>::size_type size_type;

        /*
         * @StatsVector: a bucket for single timestamp
         * @statsBuckets: per-timestamp bucket
         */
        private:
            typedef std::vector<StatsPair> StatsVector;

            StatsVector statsBuckets[TIMEOUT];

            /*
             * A compound iterator for Stats
             * As we have several vectors to go through, we support an iterator
             * to iterate over all the vectors.
             * It goes from older to newest elements, and keep insertion order.
             * It is a simple InputIterator implementation, good enough for our case.
             * A completely compliant InputIterator would need more care though.
             */
            struct statsBucketsIterator
                : public std::iterator<std::input_iterator_tag, typename StatsVector::value_type> {
                    /*
                     * @sv_iterator: the inner bucket vector iterator type
                     * @value_type: elements value type
                     */
                    typedef typename StatsVector::const_iterator sv_iterator;
                    typedef typename sv_iterator::value_type value_type;

                    /*
                     * @current: the current inner bucket vector iterator
                     * @stats: the Stats object we iterate on
                     * @ts_min: minimum timestamp to consider. As the buckets recycling occurs
                     *          only on inserts (see Stats.add() below), some old elements may
                     *          linger from some time. We must ignore them when iterating.
                     * @index: the current bucket index
                     * @index_max: the last bucket index, were we have to stop iterating
                     */
                    sv_iterator current;
                    const Stats *stats;
                    timestamp_type ts_min;
                    int index;
                    int index_max;

                    /*
                     * On intialization, we need to determine the min and max
                     * timestamps, in order to know where to start and end
                     *
                     * @stats: Stats object to iterate on
                     */
                    statsBucketsIterator(const Stats *stats)
                        : stats(stats), index(0), index_max(0)
                    {
                        timestamp_type max = 0;
                        for (int i=0; i<TIMEOUT; ++i) {
                            if (!stats->statsBuckets[i].empty()
                                    && stats->statsBuckets[i][0].first > max) {
                                max = stats->statsBuckets[i][0].first;
                                index_max = i;
                            }
                        }
                        ts_min = max - TIMEOUT;
                    }

                    /*
                     * utility functions for circular increment for
                     * buckets index
                     *
                     * @index: current bucket index
                     *
                     * @return: next buckets
                     */
                    int next_(int index)
                    {
                        return (index + 1) % TIMEOUT;
                    }

                    /*
                     * utility function to jump from the end of a vector
                     * to the begining of the next one
                     *
                     * @return: None
                     */
                    void seek()
                    {
                        while ((current == stats->statsBuckets[index].end()
                                    || current->first < ts_min)
                                && index != index_max) {
                            index = next_(index);
                            current = stats->statsBuckets[index].begin();
                        }
                    }

                    /*
                     * initialize the iterator from 1st (older) element
                     *
                     * @return: a const iterator starting from older element
                     */
                    const statsBucketsIterator& begin()
                    {
                        index = next_(index_max);
                        current = stats->statsBuckets[index].begin();
                        seek();
                        return *this;
                    }

                    /*
                     * initialize the iterator past the last (newest) element
                     *
                     * @return: a const iterator past the newest element
                     */
                    const statsBucketsIterator& end()
                    {
                        index = index_max;
                        current = stats->statsBuckets[index].end();
                        return *this;
                    }

                    const statsBucketsIterator& operator++ ()
                    {
                        ++current;
                        seek();
                        return *this;
                    }

                    const value_type& operator* () const
                    {
                        return *current;
                    }

                    const sv_iterator operator-> () const
                    {
                        return current;
                    }

                    bool operator!= (const statsBucketsIterator& other) const {
                        return this->current != other.current;
                    }

            };

        public:
            /*
             * @const_iterator: Stats elements const iterator
             */
            typedef statsBucketsIterator const_iterator;

            /*
             * get an iterator on the oldest Stats element
             *
             * @return: const iterator on the oldest Stats element
             */
            const_iterator begin() const
            {
                return statsBucketsIterator(this).begin();
            }

            /*
             * get an iterator past the newest Stats element
             *
             * @return: const iterator past the newest Stats element
             */
            const_iterator end() const
            {
                return statsBucketsIterator(this).end();
            }

            /*
             * return the number of elements in Stats
             *
             * @return: the number of elements in Stats
             */
            size_type size() const
            {
                size_type sz = 0;
                for (int i=0; i<TIMEOUT; ++i) {
                    sz += statsBuckets[i].size();
                }
                return sz;
            }

            /*
             * clear elements of Stats
             *
             * @return: None
             */
            void clear()
            {
                for (int i=0; i<TIMEOUT; ++i) {
                    statsBuckets[i].clear();
                }
            }

            /*
             * add a new (timestamp, value) pair
             *
             * @statsPair: std::pair<>(timestamp, value)
             *
             * @return: Stats
             * @complexity: O(1) (amortized)
             */
            Stats& add(StatsPair statsPair)
            {
                timestamp_type ts = statsPair.first;
                int index = ts % TIMEOUT;
                StatsVector& bucket = statsBuckets[index];
                if (!bucket.empty() && ts != bucket[0].first) bucket.clear();
                bucket.push_back(statsPair);
                return *this;
            }

            /*
             * add a new (timestamp, value) pair
             *
             * @ts: timestamp
             * @val: value
             *
             * @return: Stats
             * @complexity: O(1) (amortized)
             */
            Stats& add(timestamp_type ts, value_type val)
            {
                return add(std::make_pair(ts, val));
            }

            /*
             * add a new value, automatically timestamping it with current
             * timestamp
             *
             * @val: value
             *
             * @return: Stats
             * @complexity: O(1) (amortized)
             */
            Stats& add(value_type val)
            {
                return add(GETTIMESTAMP()(), val);
            }

            /*
             * return a const iterator over all valid Stats elements
             * valid Stats elements are the latest 60s elements
             * the iterator iterates on timestamp order, preserving
             * insertion order
             *
             * @return: const iterator over all valid Stats elements
             */
            const_iterator get_all() const
            {
                return begin();
            }

            /*
             * get the percentile of valid Stats elements
             * valid Stats elements are the latest 60s elements
             *
             * @p: percentile in % (ie 50 means median)
             *
             * @return: percentile
             * @throw: std::out_of_range when Stats is empty
             * @complexity: O(N+N^2) worst case
             *              O(2*N) average case
             */
            value_type get_p(int p) const
            {
                size_type sz = size();
                if (0 == sz) throw std::out_of_range("Stats object is empty"); 
                std::vector<T> v(sz);
                std::transform(begin(), end(), v.begin(), [](const StatsPair& sp){return sp.second;});
                size_type index = (sz * p + 99) / 100;
                std::nth_element(v.begin(), v.begin() + index, v.end());
                return v[index];
            }

            /*
             * get the 70-percentile of valid Stats elements
             * valid Stats elements are the latest 60s elements
             *
             * @return: 70-percentile
             * @throw: std::out_of_range when Stats is empty
             * @complexity: O(N+N^2) worst case
             *              O(2*N) average case
             */
            value_type get_p70() const
            {
                return get_p(70);
            }
    };

}

#endif  /* FR_BENOU_STATS_H_ */
