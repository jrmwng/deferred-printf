#pragma once

/// @file deferred_printf.h
/// @brief deferred execution of printf-like functions
/// @author jrmwng

#include <functional> // for std::function
#include <tuple> // for std::tuple, std::apply
#include <cstdarg> // for va_list, va_start, va_end
#include <stdexcept> // for std::bad_alloc
#include <type_traits> // for std::conditional_t

namespace jrmwng
{
    /**
     * @brief Abstract base class for deferred log entries.
     */
    struct Ideferred_printf_log
    {
        virtual ~Ideferred_printf_log() = default;

        /**
         * @brief Applies the provided vprintf-like function to the log entry.
         * 
         * @param fnVprintf The vprintf-like function.
         * @param pcFormat The format string.
         * @param ... The arguments for the format string.
         * @return int The result of the vprintf-like function.
         */
        static int apply_helper(std::function<int(char const *, va_list)> const &fnVprintf, char const *pcFormat, ...);

        /**
         * @brief Pure virtual function to apply the provided vprintf-like function to the log entry.
         * 
         * @param fnVprintf The vprintf-like function.
         * @return int The result of the vprintf-like function.
         */
        virtual int apply(std::function<int(char const *, va_list)> const &fnVprintf) const = 0;

        /**
         * @brief Applies the provided vprintf-like function with additional parameters to the log entry.
         * 
         * @tparam Tparams The types of the additional parameters.
         * @param pfnVprintf The vprintf-like function.
         * @param tParams The additional parameters.
         * @return int The result of the vprintf-like function.
         */
        template <typename... Tparams>
        int apply(int(*pfnVprintf)(Tparams ..., char const *, va_list), Tparams ... tParams) const
        {
            std::function<int(char const *, va_list)> const fnVprintf = [pfnVprintf, tParams...](char const *pcFormat, va_list vaArgs)
            {
                return pfnVprintf(tParams..., pcFormat, vaArgs);
            };
            return this->apply(fnVprintf);
        }

        /**
         * @brief Pure virtual function to get the size of the log entry.
         * 
         * @return size_t The size of the log entry.
         */
        virtual size_t size() const = 0;
    };

    /**
     * @brief Template class for deferred log entries.
     * 
     * @tparam Ttokens The types of the tokens.
     */
    template <typename... Ttokens>
    class Cdeferred_printf_log : public Ideferred_printf_log
    {
        static_assert(static_cast<Ideferred_printf_log*>(static_cast<Cdeferred_printf_log<Ttokens...>*>(nullptr)) == nullptr, "Cdeferred_printf_log<Ttokens...> must be derived from Ideferred_printf_log");

        std::tuple<Ttokens...> m_tupleToken;
    public:
        /**
         * @brief Constructor that initializes the log entry with the provided tokens.
         * 
         * @param tTokens The tokens.
         */
        Cdeferred_printf_log(Ttokens... tTokens) noexcept
            : m_tupleToken(tTokens...)
        {}

        /**
         * @brief Destructor.
         */
        ~Cdeferred_printf_log() noexcept override
        {
        }

        /**
         * @brief Applies the provided vprintf-like function to the log entry.
         * 
         * @param fnVprintf The vprintf-like function.
         * @return int The result of the vprintf-like function.
         */
        int apply(std::function<int(char const *, va_list)> const &fnVprintf) const noexcept override
        {
            return std::apply(&Ideferred_printf_log::apply_helper, std::tuple_cat(std::make_tuple(fnVprintf), m_tupleToken));
        }

        /**
         * @brief Returns the size of the log entry.
         * 
         * @return size_t The size of the log entry.
         */
        size_t size() const noexcept override
        {
            return sizeof(Cdeferred_printf_log<Ttokens...>);
        }
    };

    /**
     * @brief Iterator class for iterating over deferred log entries.
     */
    template <typename Tchar>
    class deferred_printf_log_iterator
    {
        Tchar *m_pBuffer;
    public:
        using reference = std::conditional_t<std::is_const_v<Tchar>, Ideferred_printf_log const &, Ideferred_printf_log &>;

        /**
         * @brief Constructor that initializes the iterator with the provided buffer and buffer end.
         * 
         * @param pcBuffer The buffer.
         */
        deferred_printf_log_iterator(Tchar *pcBuffer) noexcept;

        /**
         * @brief Inequality operator.
         * 
         * @param other The other iterator.
         * @return bool True if the iterators are not equal, false otherwise.
         */
        bool operator!=(const deferred_printf_log_iterator<Tchar> &other) const noexcept;

        /**
         * @brief Pre-increment operator.
         * 
         * @return deferred_printf_log_iterator& The incremented iterator.
         */
        deferred_printf_log_iterator<Tchar> &operator++() noexcept;

        /**
         * @brief Dereference operator.
         * 
         * @return Ideferred_printf_log& The log entry.
         */
        reference operator*() const noexcept;
    };

    /**
     * @brief Template class for logging deferred log entries.
     * 
     * @tparam zuCAPACITY The capacity of the logger.
     */
    template <size_t zuCAPACITY = 4000>
    class deferred_printf_logger
    {
        char m_acBuffer[zuCAPACITY];
        size_t m_zuLength = 0;
    public:
        /**
         * @brief Destructor that destroys all log entries.
         */
        ~deferred_printf_logger() noexcept
        {
            for (Ideferred_printf_log & iLog : *this)
            {
                iLog.~Ideferred_printf_log();
            }
        }
        
        /**
         * @brief Logs a new entry with the provided tokens.
         * 
         * @tparam Ttokens The types of the tokens.
         * @param tTokens The tokens.
         */
        template <typename... Ttokens>
        void log(Ttokens ... tTokens)
        {
            using Tlog = Cdeferred_printf_log<Ttokens...>;

            if (m_zuLength + sizeof(Tlog) <= zuCAPACITY)
            {
                new (m_acBuffer + m_zuLength) Tlog(tTokens...);
                m_zuLength += sizeof(Tlog);
            }
            else
            {
                throw std::bad_alloc();
            }
        }

        /**
         * @brief Returns an iterator to the beginning of the log entries.
         * 
         * @return deferred_printf_log_iterator<char> The iterator.
         */
        deferred_printf_log_iterator<char> begin() noexcept
        {
            return deferred_printf_log_iterator<char>{m_acBuffer};
        }

        /**
         * @brief Returns an iterator to the beginning of the log entries.
         * 
         * @return deferred_printf_log_iterator<char const> The iterator.
         */
        deferred_printf_log_iterator<char const> begin() const noexcept
        {
            return deferred_printf_log_iterator<char const>{m_acBuffer};
        }

        /**
         * @brief Returns an iterator to the end of the log entries.
         * 
         * @return deferred_printf_log_iterator<char> The iterator.
         */
        deferred_printf_log_iterator<char> end() noexcept
        {
            return deferred_printf_log_iterator<char>{m_acBuffer + m_zuLength};
        }

        /**
         * @brief Returns an iterator to the end of the log entries.
         * 
         * @return deferred_printf_log_iterator<char const> The iterator.
         */
        deferred_printf_log_iterator<char const> end() const noexcept
        {
            return deferred_printf_log_iterator<char const>{m_acBuffer + m_zuLength};
        }
    };

    /**
     * @brief Template class for deferred printf functionality.
     * 
     * @tparam Tlogger The type of the logger.
     */
    template <typename Tlogger = deferred_printf_logger<>>
    class deferred_printf
    {
        Tlogger m_Logger;
    public:

        /**
         * @brief Logs a new entry with the provided format string and arguments.
         * 
         * @tparam Targs The types of the arguments.
         * @param pcFormat The format string.
         * @param tArgs The arguments.
         */
        template <typename... Targs>
        void operator() (char const *pcFormat, Targs ... tArgs)
        {
            m_Logger.log(pcFormat, tArgs...);
        }

        /**
         * @brief Applies the provided callback function to all log entries.
         * 
         * @param fnCallback The callback function.
         */
        void apply(std::function<int(char const *, va_list)> const & fnCallback) const noexcept
        {
            for (Ideferred_printf_log const & iLog : m_Logger)
            {
                int const nCount = iLog.apply(fnCallback);
                if (nCount < 0)
                {
                    // TODO
                }
            }
        }
    };
}