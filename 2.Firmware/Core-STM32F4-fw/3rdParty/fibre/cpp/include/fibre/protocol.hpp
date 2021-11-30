/*
see protocol.md for the protocol specification
*/

#ifndef __PROTOCOL_HPP
#define __PROTOCOL_HPP

// TODO: resolve assert
#define assert(expr)

#include <functional>
#include <limits>
#include <cmath>
//#include <stdint.h>
#include <string.h>
#include "crc.hpp"
#include "cpp_utils.hpp"

// Note that this option cannot be used to debug UART because it prints on UART
//#define DEBUG_FIBRE
#ifdef DEBUG_FIBRE
#define LOG_FIBRE(...)  do { printf(__VA_ARGS__); } while (0)
#else
#define LOG_FIBRE(...)  ((void) 0)
#endif


// Default CRC-8 Polynomial: x^8 + x^5 + x^4 + x^2 + x + 1
// Can protect a 4 byte payload against toggling of up to 5 bits
//  source: https://users.ece.cmu.edu/~koopman/crc/index.html
constexpr uint8_t CANONICAL_CRC8_POLYNOMIAL = 0x37;
constexpr uint8_t CANONICAL_CRC8_INIT = 0x42;

constexpr size_t CRC8_BLOCKSIZE = 4;

// Default CRC-16 Polynomial: 0x9eb2 x^16 + x^13 + x^12 + x^11 + x^10 + x^8 + x^6 + x^5 + x^2 + 1
// Can protect a 135 byte payload against toggling of up to 5 bits
//  source: https://users.ece.cmu.edu/~koopman/crc/index.html
// Also known as CRC-16-DNP
constexpr uint16_t CANONICAL_CRC16_POLYNOMIAL = 0x3d65;
constexpr uint16_t CANONICAL_CRC16_INIT = 0x1337;

constexpr uint8_t CANONICAL_PREFIX = 0xAA;


/* move to fibre_config.h ******************************/

typedef size_t endpoint_id_t;

struct ReceiverState
{
    endpoint_id_t endpoint_id;
    size_t length;
    uint16_t seqno_thread;
    uint16_t seqno;
    bool expect_ack;
    bool expect_response;
    bool enforce_ordering;
};

/*******************************************************/



#include <unistd.h>

constexpr uint16_t PROTOCOL_VERSION = 1;

// This value must not be larger than USB_TX_DATA_SIZE defined in usbd_cdc_if.h
constexpr uint16_t TX_BUF_SIZE = 32; // does not work with 64 for some reason
constexpr uint16_t RX_BUF_SIZE = 128; // larger values than 128 have currently no effect because of protocol limitations

// Maximum time we allocate for processing and responding to a request
constexpr uint32_t PROTOCOL_SERVER_TIMEOUT_MS = 10;


typedef struct
{
    uint16_t json_crc;
    uint16_t node_id;
    uint16_t endpoint_id;
} endpoint_ref_t;

#include <cstring>

template<typename T, typename = typename std::enable_if_t<!std::is_const<T>::value>>
inline size_t write_le(T value, uint8_t *buffer)
{
    //TODO: add static_assert that this is still a little endian machine
    std::memcpy(&buffer[0], &value, sizeof(value));
    return sizeof(value);
}

template<typename T>
typename std::enable_if_t<std::is_const<T>::value, size_t>
write_le(T value, uint8_t *buffer)
{
    return write_le<std::remove_const_t<T>>(value, buffer);
}

template<>
inline size_t write_le<float>(float value, uint8_t *buffer)
{
    static_assert(CHAR_BIT * sizeof(float) == 32, "32 bit floating point expected");
    static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 floating point expected");
    const uint32_t *value_as_uint32 = reinterpret_cast<const uint32_t *>(&value);
    return write_le<uint32_t>(*value_as_uint32, buffer);
}

template<typename T>
inline size_t read_le(T *value, const uint8_t *buffer)
{
    // TODO: add static_assert that this is still a little endian machine
    std::memcpy(value, buffer, sizeof(*value));
    return sizeof(*value);
}

template<>
inline size_t read_le<float>(float *value, const uint8_t *buffer)
{
    static_assert(CHAR_BIT * sizeof(float) == 32, "32 bit floating point expected");
    static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 floating point expected");

    return read_le(reinterpret_cast<uint32_t *>(value), buffer);
}

// @brief Reads a value of type T from the buffer.
// @param buffer    Pointer to the buffer to be read. The pointer is updated by the number of bytes that were read.
// @param length    The number of available bytes in buffer. This value is updated to subtract the bytes that were read.
template<typename T>
static inline T read_le(const uint8_t **buffer, size_t *length)
{
    T result;
    size_t cnt = read_le(&result, *buffer);
    *buffer += cnt;
    *length -= cnt;
    return result;
}

class PacketSink
{
public:
    // @brief Get the maximum packet length (aka maximum transmission unit)
    // A packet size shall take no action and return an error code if the
    // caller attempts to send an oversized packet.
    //virtual size_t get_mtu() = 0;

    // @brief Processes a packet.
    // The blocking behavior shall depend on the thread-local deadline_ms variable.
    // @return: 0 on success, otherwise a non-zero error code
    // TODO: define what happens when the packet is larger than what the implementation can handle.
    virtual int process_packet(const uint8_t *buffer, size_t length) = 0;
};

class StreamSink
{
public:
    enum ChannelType_t
    {
        CHANNEL_TYPE_USB,
        CHANNEL_TYPE_UART4,
        CHANNEL_TYPE_UART5
    };

    ChannelType_t channelType;
    // @brief Processes a chunk of bytes that is part of a continuous stream.
    // The blocking behavior shall depend on the thread-local deadline_ms variable.
    // @param processed_bytes: if not NULL, shall be incremented by the number of
    //        bytes that were consumed.
    // @return: 0 on success, otherwise a non-zero error code
    virtual int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes) = 0;

    // @brief Returns the number of bytes that can still be written to the stream.
    // Shall return SIZE_MAX if the stream has unlimited lenght.
    // TODO: deprecate
    virtual size_t get_free_space() = 0;

    /*int process_bytes(const uint8_t* buffer, size_t length) {
        size_t processed_bytes = 0;
        return process_bytes(buffer, length, &processed_bytes);
    }*/
};

class StreamSource
{
public:
    // @brief Generate a chunk of bytes that are part of a continuous stream.
    // The blocking behavior shall depend on the thread-local deadline_ms variable.
    // @param generated_bytes: if not NULL, shall be incremented by the number of
    //        bytes that were written to buffer.
    // @return: 0 on success, otherwise a non-zero error code
    virtual int get_bytes(uint8_t *buffer, size_t length, size_t *generated_bytes) = 0;

    // @brief Returns the number of bytes that can still be written to the stream.
    // Shall return SIZE_MAX if the stream has unlimited lenght.
    // TODO: deprecate
    //virtual size_t get_free_space() = 0;
};

class StreamToPacketSegmenter : public StreamSink
{
public:
    StreamToPacketSegmenter(PacketSink &output) :
            output_(output)
    {
    };

    int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes);

    size_t get_free_space()
    { return SIZE_MAX; }

private:
    uint8_t header_buffer_[3];
    size_t header_index_ = 0;
    uint8_t packet_buffer_[RX_BUF_SIZE];
    size_t packet_index_ = 0;
    size_t packet_length_ = 0;
    PacketSink &output_;
};


class StreamBasedPacketSink : public PacketSink
{
public:
    StreamBasedPacketSink(StreamSink &output) :
            output_(output)
    {
    };

    //size_t get_mtu() { return SIZE_MAX; }
    int process_packet(const uint8_t *buffer, size_t length);

private:
    StreamSink &output_;
};

// @brief: Represents a stream sink that's based on an underlying packet sink.
// A single call to process_bytes may result in multiple packets being sent.
class PacketBasedStreamSink : public StreamSink
{
public:
    PacketBasedStreamSink(PacketSink &packet_sink) : _packet_sink(packet_sink)
    {}

    ~PacketBasedStreamSink()
    {}

    int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes)
    {
        // Loop to ensure all bytes get sent
        while (length)
        {
            size_t chunk = length;
            // send chunk as packet
            if (_packet_sink.process_packet(buffer, chunk))
                return -1;
            buffer += chunk;
            length -= chunk;
            if (processed_bytes)
                *processed_bytes += chunk;
        }
        return 0;
    }

    size_t get_free_space()
    { return SIZE_MAX; }

private:
    PacketSink &_packet_sink;
};

// Implements the StreamSink interface by writing into a fixed size
// memory buffer.
class MemoryStreamSink : public StreamSink
{
public:
    MemoryStreamSink(uint8_t *buffer, size_t length) :
            buffer_(buffer),
            buffer_length_(length)
    {}

    // Returns 0 on success and -1 if the buffer could not accept everything because it became full
    int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes)
    {
        size_t chunk = length < buffer_length_ ? length : buffer_length_;
        memcpy(buffer_, buffer, chunk);
        buffer_ += chunk;
        buffer_length_ -= chunk;
        if (processed_bytes)
            *processed_bytes += chunk;
        return chunk == length ? 0 : -1;
    }

    size_t get_free_space()
    { return buffer_length_; }

private:
    uint8_t *buffer_;
    size_t buffer_length_;
};

// Implements the StreamSink interface by discarding the first couple of bytes
// and then forwarding the rest to another stream.
class NullStreamSink : public StreamSink
{
public:
    NullStreamSink(size_t skip, StreamSink &follow_up_stream) :
            skip_(skip),
            follow_up_stream_(follow_up_stream)
    {}

    // Returns 0 on success and -1 if the buffer could not accept everything because it became full
    int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes)
    {
        if (skip_ < length)
        {
            buffer += skip_;
            length -= skip_;
            if (processed_bytes)
                *processed_bytes += skip_;
            skip_ = 0;
            return follow_up_stream_.process_bytes(buffer, length, processed_bytes);
        } else
        {
            skip_ -= length;
            if (processed_bytes)
                *processed_bytes += length;
            return 0;
        }
    }

    size_t get_free_space()
    { return skip_ + follow_up_stream_.get_free_space(); }

private:
    size_t skip_;
    StreamSink &follow_up_stream_;
};


// Implements the StreamSink interface by calculating the CRC16 checksum
// on the data that is sent to it.
class CRC16Calculator : public StreamSink
{
public:
    CRC16Calculator(uint16_t crc16_init) :
            crc16_(crc16_init)
    {}

    int process_bytes(const uint8_t *buffer, size_t length, size_t *processed_bytes)
    {
        crc16_ = calc_crc16<CANONICAL_CRC16_POLYNOMIAL>(crc16_, buffer, length);
        if (processed_bytes)
            *processed_bytes += length;
        return 0;
    }

    size_t get_free_space()
    { return SIZE_MAX; }

    uint16_t get_crc16()
    { return crc16_; }

private:
    uint16_t crc16_;
};


// @brief Endpoint request handler
//
// When passed a valid endpoint context, implementing functions shall handle an
// endpoint read/write request by reading the provided input data and filling in
// output data. The exact semantics of this function depends on the corresponding
// endpoint's specification.
//
// @param input: pointer to the input data
// @param input_length: number of available input bytes
// @param output: The stream where to write the output to. Can be null.
//                The handler shall abort as soon as the stream returns
//                a non-zero error code on write.
typedef std::function<void(void *ctx, const uint8_t *input, size_t input_length, StreamSink *output)> EndpointHandler;


// @brief Default endpoint handler for const types
// @return: True if endpoint was written to, False otherwise
template<typename T>
std::enable_if_t<!std::is_same<T, endpoint_ref_t>::value && std::is_const<T>::value, bool>
default_readwrite_endpoint_handler(T *value, const uint8_t *input, size_t input_length, StreamSink *output)
{
    // If the old value was requested, call the corresponding little endian serialization function
    if (output)
    {
        // TODO: make buffer size dependent on the type
        uint8_t buffer[sizeof(T)];
        size_t cnt = write_le<T>(*value, buffer);
        if (cnt <= output->get_free_space())
            output->process_bytes(buffer, cnt, nullptr);
    }
    return false; // We don't ever write to const types
}

// @brief Default endpoint handler for non-const types
template<typename T>
std::enable_if_t<!std::is_same<T, endpoint_ref_t>::value && !std::is_const<T>::value, bool>
default_readwrite_endpoint_handler(T *value, const uint8_t *input, size_t input_length, StreamSink *output)
{
    // Read the endpoint value into output
    default_readwrite_endpoint_handler<const T>(const_cast<const T *>(value), input, input_length, output);

    // If a new value was passed, call the corresponding little endian deserialization function
    uint8_t buffer[sizeof(T)] = {0}; // TODO: make buffer size dependent on the type
    if (input_length >= sizeof(buffer))
    {
        read_le<T>(value, input);
        return true;
    } else
    {
        return false;
    }
}

// @brief Default endpoint handler for endpoint_ref_t types
template<typename T>
bool
default_readwrite_endpoint_handler(endpoint_ref_t *value, const uint8_t *input, size_t input_length, StreamSink *output)
{
    constexpr size_t size = sizeof(value->endpoint_id) + sizeof(value->json_crc);
    if (output)
    {
        // TODO: make buffer size dependent on the type
        uint8_t buffer[size];
        size_t cnt = write_le<decltype(value->endpoint_id)>(value->endpoint_id, buffer);
        cnt += write_le<decltype(value->json_crc)>(value->json_crc, buffer + cnt);
        if (cnt <= output->get_free_space())
            output->process_bytes(buffer, cnt, nullptr);
    }

    // If a new value was passed, call the corresponding little endian deserialization function
    if (input_length >= size)
    {
        read_le<decltype(value->endpoint_id)>(&value->endpoint_id, input);
        read_le<decltype(value->json_crc)>(&value->json_crc, input + 2);
        return true;
    } else
    {
        return false;
    }
}

template<typename T>
static inline const char *get_default_json_modifier();

template<>
inline constexpr const char *get_default_json_modifier<const float>()
{
    return "\"type\":\"float\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<float>()
{
    return "\"type\":\"float\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const int64_t>()
{
    return "\"type\":\"int64\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<int64_t>()
{
    return "\"type\":\"int64\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const uint64_t>()
{
    return "\"type\":\"uint64\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<uint64_t>()
{
    return "\"type\":\"uint64\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const int32_t>()
{
    return "\"type\":\"int32\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<int32_t>()
{
    return "\"type\":\"int32\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const uint32_t>()
{
    return "\"type\":\"uint32\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<uint32_t>()
{
    return "\"type\":\"uint32\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const uint16_t>()
{
    return "\"type\":\"uint16\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<uint16_t>()
{
    return "\"type\":\"uint16\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const uint8_t>()
{
    return "\"type\":\"uint8\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<uint8_t>()
{
    return "\"type\":\"uint8\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<const bool>()
{
    return "\"type\":\"bool\",\"access\":\"r\"";
}

template<>
inline constexpr const char *get_default_json_modifier<bool>()
{
    return "\"type\":\"bool\",\"access\":\"rw\"";
}

template<>
inline constexpr const char *get_default_json_modifier<endpoint_ref_t>()
{
    return "\"type\":\"endpoint_ref\",\"access\":\"rw\"";
}

class Endpoint
{
public:
    //const char* const name_;
    virtual void handle(const uint8_t *input, size_t input_length, StreamSink *output) = 0;

    virtual bool get_string(char *output, size_t length)
    { return false; }

    virtual bool set_string(char *buffer, size_t length)
    { return false; }

    virtual bool set_from_float(float value)
    { return false; }
};

static inline int write_string(const char *str, StreamSink *output)
{
    return output->process_bytes(reinterpret_cast<const uint8_t *>(str), strlen(str), nullptr);
}


/* @brief Handles the communication protocol on one channel.
*
* When instantiated with a list of endpoints and an output packet sink,
* objects of this class will handle packets passed into process_packet,
* pass the relevant data to the corresponding endpoints and dispatch response
* packets on the output.
*/
class BidirectionalPacketBasedChannel : public PacketSink
{
public:
    BidirectionalPacketBasedChannel(PacketSink &output) :
            output_(output)
    {}

    //size_t get_mtu() {
    //    return SIZE_MAX;
    //}
    int process_packet(const uint8_t *buffer, size_t length);

private:
    PacketSink &output_;
    uint8_t tx_buf_[TX_BUF_SIZE];
};


/* ToString / FromString functions -------------------------------------------*/
/*
* These functions are currently not used by Fibre and only here to
* support the ODrive ASCII protocol.
* TODO: find a general way for client code to augment endpoints with custom
* functions
*/

template<typename T>
struct format_traits_t;

// template<> struct format_traits_t<float> { using type = void;
//     static constexpr const char * fmt = "%f";
//     static constexpr const char * fmtp = "%f";
// };
template<>
struct format_traits_t<int64_t>
{
    using type = void;
    static constexpr const char *fmt = "%lld";
    static constexpr const char *fmtp = "%lld";
};
template<>
struct format_traits_t<uint64_t>
{
    using type = void;
    static constexpr const char *fmt = "%llu";
    static constexpr const char *fmtp = "%llu";
};
template<>
struct format_traits_t<int32_t>
{
    using type = void;
    static constexpr const char *fmt = "%ld";
    static constexpr const char *fmtp = "%ld";
};
template<>
struct format_traits_t<uint32_t>
{
    using type = void;
    static constexpr const char *fmt = "%lu";
    static constexpr const char *fmtp = "%lu";
};
template<>
struct format_traits_t<int16_t>
{
    using type = void;
    static constexpr const char *fmt = "%hd";
    static constexpr const char *fmtp = "%hd";
};
template<>
struct format_traits_t<uint16_t>
{
    using type = void;
    static constexpr const char *fmt = "%hu";
    static constexpr const char *fmtp = "%hu";
};
template<>
struct format_traits_t<int8_t>
{
    using type = void;
    static constexpr const char *fmt = "%hhd";
    static constexpr const char *fmtp = "%d";
};
template<>
struct format_traits_t<uint8_t>
{
    using type = void;
    static constexpr const char *fmt = "%hhu";
    static constexpr const char *fmtp = "%u";
};

template<typename T, typename = typename format_traits_t<T>::type>
static bool to_string(const T &value, char *buffer, size_t length, int)
{
    snprintf(buffer, length, format_traits_t<T>::fmtp, value);
    return true;
}

// Special case for float because printf promotes float to double, and we get warnings
template<typename T = float>
static bool to_string(const float &value, char *buffer, size_t length, int)
{
    snprintf(buffer, length, "%f", (double) value);
    return true;
}

template<typename T = bool>
static bool to_string(const bool &value, char *buffer, size_t length, int)
{
    buffer[0] = value ? '1' : '0';
    buffer[1] = 0;
    return true;
}

template<typename T>
static bool to_string(const T &value, char *buffer, size_t length, ...)
{
    return false;
}

template<typename T, typename = typename format_traits_t<T>::type>
static bool from_string(const char *buffer, size_t length, T *property, int)
{
    return sscanf(buffer, format_traits_t<T>::fmt, property) == 1;
}

// Special case for float because printf promotes float to double, and we get warnings
template<typename T = float>
static bool from_string(const char *buffer, size_t length, float *property, int)
{
    return sscanf(buffer, "%f", property) == 1;
}

template<typename T = bool>
static bool from_string(const char *buffer, size_t length, bool *property, int)
{
    int val;
    if (sscanf(buffer, "%d", &val) != 1)
        return false;
    *property = val;
    return true;
}

template<typename T>
static bool from_string(const char *buffer, size_t length, T *property, ...)
{
    return false;
}


/* Object tree ---------------------------------------------------------------*/

template<typename ... TMembers>
struct MemberList;

template<>
struct MemberList<>
{
public:
    static constexpr size_t endpoint_count = 0;
    static constexpr bool is_empty = true;

    void write_json(size_t id, StreamSink *output)
    {
        // no action
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length)
    {
        // no action
    }

    Endpoint *get_by_name(const char *name, size_t length)
    {
        return nullptr;
    }

    std::tuple<> get_names_as_tuple() const
    { return std::tuple<>(); }
};

template<typename TMember, typename ... TMembers>
struct MemberList<TMember, TMembers...>
{
public:
    static constexpr size_t endpoint_count = TMember::endpoint_count + MemberList<TMembers...>::endpoint_count;
    static constexpr bool is_empty = false;

    MemberList(TMember &&this_member, TMembers &&... subsequent_members) :
            this_member_(std::forward<TMember>(this_member)),
            subsequent_members_(std::forward<TMembers>(subsequent_members)...)
    {}

    MemberList(TMember &&this_member, MemberList<TMembers...> &&subsequent_members) :
            this_member_(std::forward<TMember>(this_member)),
            subsequent_members_(std::forward<MemberList<TMembers...>>(subsequent_members))
    {}

    // @brief Move constructor
/*    MemberList(MemberList&& other) :
        this_member_(std::move(other.this_member_)),
        subsequent_members_(std::move(other.subsequent_members_)) {}*/

    void write_json(size_t id, StreamSink *output) /*final*/ {
        this_member_.write_json(id, output);
        if (!MemberList<TMembers...>::is_empty)
            write_string(",", output);
        subsequent_members_.write_json(id + TMember::endpoint_count, output);
    }

    Endpoint *get_by_name(const char *name, size_t length)
    {
        Endpoint *result = this_member_.get_by_name(name, length);
        if (result) return result;
        else return subsequent_members_.get_by_name(name, length);
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length) /*final*/ {
        this_member_.register_endpoints(list, id, length);
        subsequent_members_.register_endpoints(list, id + TMember::endpoint_count, length);
    }

    TMember this_member_;
    MemberList<TMembers...> subsequent_members_;
};

template<typename ... TMembers>
MemberList<TMembers...> make_protocol_member_list(TMembers &&... member_list)
{
    return MemberList<TMembers...>(std::forward<TMembers>(member_list)...);
}

template<typename ... TMembers>
class ProtocolObject
{
public:
    ProtocolObject(const char *name, TMembers &&... member_list) :
            name_(name),
            member_list_(std::forward<TMembers>(member_list)...)
    {}

    static constexpr size_t endpoint_count = MemberList<TMembers...>::endpoint_count;

    void write_json(size_t id, StreamSink *output)
    {
        write_string("{\"name\":\"", output);
        write_string(name_, output);
        write_string("\",\"type\":\"object\",\"members\":[", output);
        member_list_.write_json(id, output),
                write_string("]}", output);
    }

    Endpoint *get_by_name(const char *name, size_t length)
    {
        size_t segment_length = strlen(name);
        if (!strncmp(name, name_, length))
            return member_list_.get_by_name(name + segment_length + 1, length - segment_length - 1);
        else
            return nullptr;
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length)
    {
        member_list_.register_endpoints(list, id, length);
    }

    const char *name_;
    MemberList<TMembers...> member_list_;
};

template<typename ... TMembers>
ProtocolObject<TMembers...> make_protocol_object(const char *name, TMembers &&... member_list)
{
    return ProtocolObject<TMembers...>(name, std::forward<TMembers>(member_list)...);
}

//template<typename T, typename = typename std>
//bool set_from_float_ex(float value, T* property) {
//    return false;
//}

namespace conversion
{
//template<typename T>
    template<typename T>
    bool set_from_float_ex(float value, float *property, int)
    {
        return *property = value, true;
    }

    template<typename T>
    bool set_from_float_ex(float value, bool *property, int)
    {
        return *property = (value >= 0.0f), true;
    }

    template<typename T, typename = std::enable_if_t<std::is_integral<T>::value && !std::is_const<T>::value>>
    bool set_from_float_ex(float value, T *property, int)
    {
        return *property = static_cast<T>(std::round(value)), true;
    }

    template<typename T>
    bool set_from_float_ex(float value, T *property, ...)
    {
        return false;
    }

    template<typename T>
    bool set_from_float(float value, T *property)
    {
        return set_from_float_ex<T>(value, property, 0);
    }
}

//template<typename T>
//bool set_from_float_ex<>(float value, T* property) {
//    return false;
//}


template<typename TProperty>
class ProtocolProperty : public Endpoint
{
public:
    static constexpr const char *json_modifier = get_default_json_modifier<TProperty>();
    static constexpr size_t endpoint_count = 1;

    ProtocolProperty(const char *name, TProperty *property,
                     void (*written_hook)(void *), void *ctx)
            : name_(name), property_(property), written_hook_(written_hook), ctx_(ctx)
    {}

/*  TODO: find out why the move constructor is not used when it could be
    ProtocolProperty(const ProtocolProperty&) = delete;
    // @brief Move constructor
    ProtocolProperty(ProtocolProperty&& other) :
        Endpoint(std::move(other)),
        name_(std::move(other.name_)),
        property_(other.property_)
    {}
    constexpr ProtocolProperty& operator=(const ProtocolProperty& other) = delete;
    constexpr ProtocolProperty& operator=(const ProtocolProperty& other) {
        //Endpoint(std::move(other)),
        //name_(std::move(other.name_)),
        //property_(other.property_)
        name_ = other.name_;
        property_ = other.property_;
        return *this;
    }
    ProtocolProperty& operator=(ProtocolProperty&& other)
        : name_(other.name_), property_(other.property_)
    {}
    ProtocolProperty& operator=(const ProtocolProperty& other)
        : name_(other.name_), property_(other.property_)
    {}*/

    void write_json(size_t id, StreamSink *output)
    {
        // write name
        write_string("{\"name\":\"", output);
        LOG_FIBRE("json: this at %x, name at %x is s\r\n", (uintptr_t) this, (uintptr_t) name_);
        //LOG_FIBRE("json\r\n");
        write_string(name_, output);

        // write endpoint ID
        write_string("\",\"id\":", output);
        char id_buf[10];
        snprintf(id_buf, sizeof(id_buf), "%u", (unsigned) id); // TODO: get rid of printf
        write_string(id_buf, output);

        // write additional JSON data
        if (json_modifier && json_modifier[0])
        {
            write_string(",", output);
            write_string(json_modifier, output);
        }

        write_string("}", output);
    }

    // special-purpose function - to be moved
    Endpoint *get_by_name(const char *name, size_t length)
    {
        if (!strncmp(name, name_, length))
            return this;
        else
            return nullptr;
    }

    // special-purpose function - to be moved
    bool get_string(char *buffer, size_t length) final
    {
        return to_string(*property_, buffer, length, 0);
    }

    // special-purpose function - to be moved
    bool set_string(char *buffer, size_t length) final
    {
        return from_string(buffer, length, property_, 0);
    }

    bool set_from_float(float value) final
    {
        return conversion::set_from_float(value, property_);
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length)
    {
        if (id < length)
            list[id] = this;
    }

    void handle(const uint8_t *input, size_t input_length, StreamSink *output) final
    {
        bool wrote = default_readwrite_endpoint_handler<TProperty>(property_, input, input_length, output);
        if (wrote && written_hook_ != nullptr)
        {
            written_hook_(ctx_);
        }
    }

    /*void handle(const uint8_t* input, size_t input_length, StreamSink* output) {
        handle(input, input_length, output);
    }*/

    const char *name_;
    TProperty *property_;

    void (*written_hook_)(void *);

    void *ctx_;
};

// Non-const non-enum types
template<typename TProperty, ENABLE_IF(!std::is_enum<TProperty>::value) >
ProtocolProperty<TProperty> make_protocol_property(const char *name, TProperty *property,
                                                   void (*written_hook)(void *) = nullptr, void *ctx = nullptr)
{
    return ProtocolProperty<TProperty>(name, property, written_hook, ctx);
};

// Const non-enum types
template<typename TProperty, ENABLE_IF(!std::is_enum<TProperty>::value) >
ProtocolProperty<const TProperty> make_protocol_ro_property(const char *name, TProperty *property,
                                                            void (*written_hook)(void *) = nullptr, void *ctx = nullptr)
{
    return ProtocolProperty<const TProperty>(name, property, written_hook, ctx);
};

// Non-const enum types
template<typename TProperty, ENABLE_IF(std::is_enum<TProperty>::value) >
ProtocolProperty<std::underlying_type_t<TProperty>> make_protocol_property(const char *name, TProperty *property,
                                                                           void (*written_hook)(void *) = nullptr,
                                                                           void *ctx = nullptr)
{
    return ProtocolProperty<std::underlying_type_t<TProperty>>(
            name, reinterpret_cast<std::underlying_type_t<TProperty> *>(property), written_hook, ctx);
};

// Const enum types
template<typename TProperty, ENABLE_IF(std::is_enum<TProperty>::value) >
ProtocolProperty<const std::underlying_type_t<TProperty>>
make_protocol_ro_property(const char *name, TProperty *property,
                          void (*written_hook)(void *) = nullptr, void *ctx = nullptr)
{
    return ProtocolProperty<const std::underlying_type_t<TProperty>>(
            name, reinterpret_cast<const std::underlying_type_t<TProperty> *>(property), written_hook, ctx);
};


template<typename ... TArgs>
struct PropertyListFactory;

template<>
struct PropertyListFactory<>
{
    template<unsigned IPos, typename ... TAllProperties>
    static MemberList<>
    make_property_list(std::array<const char *, sizeof...(TAllProperties)> names, std::tuple<TAllProperties...> &values)
    {
        return MemberList<>();
    }
};

template<typename TProperty, typename ... TProperties>
struct PropertyListFactory<TProperty, TProperties...>
{
    template<unsigned IPos, typename ... TAllProperties>
    static MemberList<ProtocolProperty<TProperty>, ProtocolProperty<TProperties>...>
    make_property_list(std::array<const char *, sizeof...(TAllProperties)> names, std::tuple<TAllProperties...> &values)
    {
        return MemberList<ProtocolProperty<TProperty>, ProtocolProperty<TProperties>...>(
                make_protocol_property(std::get<IPos>(names), &std::get<IPos>(values)),
                PropertyListFactory<TProperties...>::template make_property_list<IPos + 1>(names, values)
        );
    }
};

/* @brief return_type<TypeList>::type represents the true return type
* of a function returning 0 or more arguments.
*
* For an empty TypeList, the return type is void. For a list with
* one type, the return type is equal to that type. For a list with
* more than one items, the return type is a tuple.
*/
template<typename ... Types>
struct return_type;

template<>
struct return_type<>
{
    typedef void type;
};
template<typename T>
struct return_type<T>
{
    typedef T type;
};
template<typename T, typename ... Ts>
struct return_type<T, Ts...>
{
    typedef std::tuple<T, Ts...> type;
};


template<typename TObj, typename ... TInputsAndOutputs>
class ProtocolFunction;

template<typename TObj, typename ... TInputs, typename ... TOutputs>
class ProtocolFunction<TObj, std::tuple<TInputs...>, std::tuple<TOutputs...>> : Endpoint
{
public:
    // @brief The return type of the function as written by a C++ programmer
    using TRet = typename return_type<TOutputs...>::type;

    static constexpr size_t endpoint_count = 1 + MemberList<ProtocolProperty<TInputs>...>::endpoint_count +
                                             MemberList<ProtocolProperty<TOutputs>...>::endpoint_count;

    ProtocolFunction(const char *name, TObj &obj, TRet(TObj::*func_ptr)(TInputs...),
                     std::array<const char *, sizeof...(TInputs)> input_names,
                     std::array<const char *, sizeof...(TOutputs)> output_names) :
            name_(name), obj_(&obj), func_ptr_(func_ptr),
            input_names_{input_names}, output_names_{output_names},
            input_properties_(PropertyListFactory<TInputs...>::template make_property_list<0>(input_names_, in_args_)),
            output_properties_(
                    PropertyListFactory<TOutputs...>::template make_property_list<0>(output_names_, out_args_))
    {
        LOG_FIBRE("my tuple is at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
    }

    // The custom copy constructor is needed because otherwise the
    // input_properties_ and output_properties_ would point to memory
    // locations of the old object.
    ProtocolFunction(const ProtocolFunction &other) :
            name_(other.name_), obj_(other.obj_), func_ptr_(other.func_ptr_),
            input_names_{other.input_names_}, output_names_{other.output_names_},
            input_properties_(PropertyListFactory<TInputs...>::template make_property_list<0>(input_names_, in_args_)),
            output_properties_(
                    PropertyListFactory<TOutputs...>::template make_property_list<0>(output_names_, out_args_))
    {
        LOG_FIBRE("COPIED! my tuple is at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
    }

    void write_json(size_t id, StreamSink *output)
    {
        // write name
        write_string("{\"name\":\"", output);
        write_string(name_, output);

        // write endpoint ID
        write_string("\",\"id\":", output);
        char id_buf[10];
        snprintf(id_buf, sizeof(id_buf), "%u", (unsigned) id); // TODO: get rid of printf
        write_string(id_buf, output);

        // write arguments
        write_string(",\"type\":\"function\",\"inputs\":[", output);
        input_properties_.write_json(id + 1, output),
                write_string("],\"outputs\":[", output);
        output_properties_.write_json(id + 1 + decltype(input_properties_)::endpoint_count, output),
                write_string("]}", output);
    }

    // special-purpose function - to be moved
    Endpoint *get_by_name(const char *name, size_t length)
    {
        return nullptr; // can't address functions by name
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length)
    {
        if (id < length)
            list[id] = this;
        input_properties_.register_endpoints(list, id + 1, length);
        output_properties_.register_endpoints(list, id + 1 + decltype(input_properties_)::endpoint_count, length);
    }

    template<size_t i = sizeof...(TOutputs)>
    std::enable_if_t<i == 0>
    handle_ex()
    {
        invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
    }

    template<size_t i = sizeof...(TOutputs)>
    std::enable_if_t<i == 1>
    handle_ex()
    {
        std::get<0>(out_args_) = invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
    }

    template<size_t i = sizeof...(TOutputs)>
    std::enable_if_t<i >= 2>
    handle_ex()
    {
        out_args_ = invoke_function_with_tuple(*obj_, func_ptr_, in_args_);
    }

    void handle(const uint8_t *input, size_t input_length, StreamSink *output) final
    {
        (void) input;
        (void) input_length;
        (void) output;
        LOG_FIBRE("tuple still at %x and of size %u\r\n", (uintptr_t) &in_args_, sizeof(in_args_));
        LOG_FIBRE("invoke function using %d and %.3f\r\n", std::get<0>(in_args_), std::get<1>(in_args_));
        handle_ex();
    }

    const char *name_;
    TObj *obj_;

    TRet (TObj::*func_ptr_)(TInputs...);

    std::array<const char *, sizeof...(TInputs)> input_names_; // TODO: remove
    std::array<const char *, sizeof...(TOutputs)> output_names_; // TODO: remove
    std::tuple<TInputs...> in_args_;
    std::tuple<TOutputs...> out_args_;
    MemberList<ProtocolProperty<TInputs>...> input_properties_;
    MemberList<ProtocolProperty<TOutputs>...> output_properties_;
};

template<typename TObj, typename ... TArgs, typename ... TNames,
        typename = std::enable_if_t<sizeof...(TArgs) == sizeof...(TNames)>>
ProtocolFunction<TObj, std::tuple<TArgs...>, std::tuple<>>
make_protocol_function(const char *name, TObj &obj, void(TObj::*func_ptr)(TArgs...), TNames ... names)
{
    return ProtocolFunction<TObj, std::tuple<TArgs...>, std::tuple<>>(name, obj, func_ptr, {names...}, {});
}

template<typename TObj, typename TRet, typename ... TArgs, typename ... TNames,
        typename = std::enable_if_t<sizeof...(TArgs) == sizeof...(TNames) && !std::is_void<TRet>::value>>
ProtocolFunction<TObj, std::tuple<TArgs...>, std::tuple<TRet>>
make_protocol_function(const char *name, TObj &obj, TRet(TObj::*func_ptr)(TArgs...), TNames ... names)
{
    return ProtocolFunction<TObj, std::tuple<TArgs...>, std::tuple<TRet>>(name, obj, func_ptr, {names...}, {"result"});
}


#define FIBRE_EXPORTS(CLASS, ...) \
    struct fibre_export_t { \
        static CLASS* obj; \
        using type = decltype(make_protocol_member_list(__VA_ARGS__)); \
    }; \
    fibre_export_t::type make_fibre_definitions() { \
        CLASS* obj = this; \
        return make_protocol_member_list(__VA_ARGS__); \
    } \
    fibre_export_t::type fibre_definitions = make_fibre_definitions()


class EndpointProvider
{
public:
    virtual size_t get_endpoint_count() = 0;

    virtual void write_json(size_t id, StreamSink *output) = 0;

    virtual Endpoint *get_by_name(char *name, size_t length) = 0;

    virtual void register_endpoints(Endpoint **list, size_t id, size_t length) = 0;
};

template<typename T>
class EndpointProvider_from_MemberList : public EndpointProvider
{
public:
    EndpointProvider_from_MemberList(T &member_list) : member_list_(member_list)
    {}

    size_t get_endpoint_count() final
    {
        return T::endpoint_count;
    }

    void write_json(size_t id, StreamSink *output) final
    {
        return member_list_.write_json(id, output);
    }

    void register_endpoints(Endpoint **list, size_t id, size_t length) final
    {
        return member_list_.register_endpoints(list, id, length);
    }

    Endpoint *get_by_name(char *name, size_t length) final
    {
        for (size_t i = 0; i < length; i++)
        {
            if (name[i] == '.')
                name[i] = 0;
        }
        name[length - 1] = 0;
        return member_list_.get_by_name(name, length);
    }

    T &member_list_;
};


class JSONDescriptorEndpoint : Endpoint
{
public:
    static constexpr size_t endpoint_count = 1;

    void write_json(size_t id, StreamSink *output);

    void register_endpoints(Endpoint **list, size_t id, size_t length);

    void handle(const uint8_t *input, size_t input_length, StreamSink *output);
};

// defined in protocol.cpp
extern Endpoint **endpoint_list_;
extern size_t n_endpoints_;
extern uint16_t json_crc_;
extern JSONDescriptorEndpoint json_file_endpoint_;
extern EndpointProvider *application_endpoints_;

bool is_endpoint_ref_valid(endpoint_ref_t endpoint_ref);

Endpoint *get_endpoint(endpoint_ref_t endpoint_ref);

// @brief Registers the specified application object list using the provided endpoint table.
// This function should only be called once during the lifetime of the application. TODO: fix this.
// @param application_objects The application objects to be registred.
template<typename T>
int fibre_publish(T &application_objects)
{
    static constexpr size_t endpoint_list_size = 1 + T::endpoint_count;
    static Endpoint *endpoint_list[endpoint_list_size];
    static auto endpoint_provider = EndpointProvider_from_MemberList<T>(application_objects);

    json_file_endpoint_.register_endpoints(endpoint_list, 0, endpoint_list_size);
    application_objects.register_endpoints(endpoint_list, 1, endpoint_list_size);

    // Update the global endpoint table
    endpoint_list_ = endpoint_list;
    n_endpoints_ = endpoint_list_size;
    application_endpoints_ = &endpoint_provider;

    // Calculate the CRC16 of the JSON file.
    // The init value is the protocol version.
    CRC16Calculator crc16_calculator(PROTOCOL_VERSION);
    uint8_t offset[4] = {0};
    json_file_endpoint_.handle(offset, sizeof(offset), &crc16_calculator);
    json_crc_ = crc16_calculator.get_crc16();

    return 0;
}


#endif
