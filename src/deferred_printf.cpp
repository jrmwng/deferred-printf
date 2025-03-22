#include "deferred_printf.h"

namespace jrmwng
{
    /**
     * @brief Constructs a deferred_printf_log_iterator with the given buffer.
     * 
     * @param pcBuffer Pointer to the buffer.
     */
    template <typename Tchar>
    deferred_printf_log_iterator<Tchar>::deferred_printf_log_iterator(Tchar *pcBuffer) noexcept
        : m_pBuffer(pcBuffer)
    {
    }

    /**
     * @brief Compares two iterators for inequality.
     * 
     * @param other The other iterator to compare with.
     * @return true if the iterators are not equal, false otherwise.
     */
    template <typename Tchar>
    bool deferred_printf_log_iterator<Tchar>::operator!=(const deferred_printf_log_iterator<Tchar> &other) const noexcept
    {
        return m_pBuffer != other.m_pBuffer;
    }

    /**
     * @brief Advances the iterator to the next log entry.
     * 
     * @return A reference to the updated iterator.
     */
    template <typename Tchar>
    deferred_printf_log_iterator<Tchar> &deferred_printf_log_iterator<Tchar>::operator++() noexcept
    {
        m_pBuffer += reinterpret_cast<Ideferred_printf_log const *>(m_pBuffer)->size();
        return *this;
    }

    /**
     * @brief Dereferences the iterator to access the current log entry.
     * 
     * @return A reference to the current log entry.
     */
    template <typename Tchar>
    typename deferred_printf_log_iterator<Tchar>::reference deferred_printf_log_iterator<Tchar>::operator*() const noexcept
    {
        return *reinterpret_cast<std::conditional_t<std::is_const_v<Tchar>, Ideferred_printf_log const, Ideferred_printf_log> *>(m_pBuffer);
    }

    template class Cdeferred_printf_log<char const *>;

    // Explicit instantiation of deferred_printf_log_iterator for char and char const types
    template class deferred_printf_log_iterator<char>;
    template class deferred_printf_log_iterator<char const>;

    // Explicit instantiation of deferred_printf_logger with default capacity
    template class deferred_printf_logger<>;
}