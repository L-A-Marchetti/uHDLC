#include "../include/uHDLC.h"

uHDLC uHDLC_encode(uint8_t address, uint8_t control, const uint8_t *data, size_t data_length)
{
    uHDLC frame;
    frame.address = address;
    frame.control = control;
    frame.data_length = data_length;
    for (size_t i = 0; i < data_length; i++)
    {
        frame.data[i] = data[i];
    }
    size_t fcs_length = 2 + data_length;
    uint8_t fcs_data[fcs_length];
    fcs_data[0] = frame.address;
    fcs_data[1] = frame.control;
    for (size_t i = 0; i < data_length; i++)
    {
        fcs_data[2 + i] = frame.data[i];
    }
    frame.fcs = uHDLC_fcs(fcs_data, fcs_length);
    uint8_t stuffed_data[data_length*2];
    size_t stuffed_data_length;
    uHDLC_bs(data, data_length, stuffed_data, &stuffed_data_length);
    size_t full_frame_length = 1 + 1 + 1 + stuffed_data_length + 2 + 1;
    if (full_frame_length <= MAX_FRAME_SIZE)
    {
        frame.frame[0] = HDLC_FLAG;
        frame.frame[1] = frame.address;
        frame.frame[2] = frame.control;
        for (size_t i = 0; i < stuffed_data_length; i++)
        {
            frame.frame[3 + i] = stuffed_data[i];
        }
        *((uint16_t *)&frame.frame[3 + stuffed_data_length]) = frame.fcs;
        frame.frame[full_frame_length - 1] = HDLC_FLAG;
        frame.frame_length = full_frame_length;
    }
    else
    {
        printf("Error: Frame exceeds maximum size.\n");
        frame.error = 1;
        return frame;
    }
    return frame;
}

uHDLC uHDLC_decode(const uint8_t *frame, size_t frame_length) {
    uHDLC decoded_frame;
    if (frame_length < 5 || frame[0] != HDLC_FLAG || frame[frame_length - 1] != HDLC_FLAG)
    {
        printf("Error: Invalid HDLC frame.\n");
        decoded_frame.error = 1;
        return decoded_frame;
    }
    if (frame_length <= MAX_FRAME_SIZE)
    {
        for (size_t i = 0; i < frame_length; i++)
        {
            decoded_frame.frame[i] = frame[i];
        }
        decoded_frame.frame_length = frame_length;
    }
    else
    {
        printf("Error: Frame exceeds maximum size.\n");
        decoded_frame.error = 1;
        return decoded_frame;
    }
    decoded_frame.address = frame[1];
    decoded_frame.control = frame[2];
    size_t stuffed_data_length = frame_length - 6;
    uint8_t stuffed_data[stuffed_data_length];    
    for (size_t i = 0; i < stuffed_data_length; i++)
    {
        stuffed_data[i] = frame[3 + i];
    }
    uint8_t destuffed_data[stuffed_data_length];
    size_t destuffed_data_length;
    uHDLC_bds(stuffed_data, stuffed_data_length, destuffed_data, &destuffed_data_length);
    decoded_frame.data_length = destuffed_data_length;
    for (size_t i = 0; i < decoded_frame.data_length; i++)
    {
        decoded_frame.data[i] = destuffed_data[i];
    }
    decoded_frame.fcs = *((uint16_t *)&frame[3 + stuffed_data_length]);
    uint8_t fcs_data[2 + decoded_frame.data_length];
    fcs_data[0] = decoded_frame.address;
    fcs_data[1] = decoded_frame.control;
    for (size_t i = 0; i < decoded_frame.data_length; i++)
    {
        fcs_data[2 + i] = decoded_frame.data[i];
    }
    if (decoded_frame.fcs != uHDLC_fcs(fcs_data, sizeof(fcs_data)))
    {
        printf("Error: FCS mismatch.\n");
        decoded_frame.error = 1;
        return decoded_frame;
    }
    return decoded_frame;
}

void uHDLC_bs(const uint8_t* input, size_t input_length, uint8_t* output, size_t* output_length)
{
    size_t output_index = 0;
    int consecutive_ones = 0;
    uint8_t current_byte = 0;
    int bit_position = 7;
    for (size_t i = 0; i < input_length; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            uint8_t bit = (input[i] >> j) & 1;
            current_byte |= (bit << bit_position);
            bit_position--;
            if (bit == 1)
            {
                consecutive_ones++;
                if (consecutive_ones == 5)
                {
                    bit_position--;
                    if (bit_position < 0)
                    {
                        output[output_index++] = current_byte;
                        current_byte = 0;
                        bit_position = 7;
                    }
                    consecutive_ones = 0;
                }
            }
            else
            {
                consecutive_ones = 0;
            }

            if (bit_position < 0)
            {
                output[output_index++] = current_byte;
                current_byte = 0;
                bit_position = 7;
            }
        }
    }
    if (bit_position != 7)
    {
        output[output_index++] = current_byte;
    }
    *output_length = output_index;
    if (output_index > 0 && output[output_index - 1] == 0)
    {
        (*output_length)--;
    }
}

void uHDLC_bds(const uint8_t* input, size_t input_length, uint8_t* output, size_t* output_length)
{
    size_t output_index = 0;
    int consecutive_ones = 0;
    uint8_t current_byte = 0;
    int bit_position = 7;
    for (size_t i = 0; i < input_length; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            uint8_t bit = (input[i] >> j) & 1;
            if (consecutive_ones == 5 && bit == 0)
            {
                consecutive_ones = 0;
                continue;
            }
            current_byte |= (bit << bit_position);
            bit_position--;
            if (bit == 1)
            {
                consecutive_ones++;
            }
            else
            {
                consecutive_ones = 0;
            }
            if (bit_position < 0)
            {
                output[output_index++] = current_byte;
                current_byte = 0;
                bit_position = 7;
            }
        }
    }
    if (bit_position != 7)
    {
        output[output_index++] = current_byte;
    }
    *output_length = output_index;
    if (output_index > 0 && output[output_index - 1] == 0)
    {
        (*output_length)--;
    }
}

uint16_t uHDLC_fcs(const uint8_t *data, size_t length)
{
    uint16_t fcs = 0xFFFF; // Initial value
    for (size_t i = 0; i < length; i++)
    {
        fcs = (fcs >> 8) ^ fcstab[(fcs ^ data[i]) & 0xFF];
    }
    return fcs ^ 0xFFFF; // Invert final result
}

void octet_debugging(uint8_t octet)
{
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (octet >> i) & 1);
    }
}

void frame_debugging(const uint8_t *frame, size_t size) {
    for (size_t i = 0; i < size; i++) {
        octet_debugging(frame[i]);
        printf(" ");
    }
    printf("\n");
}

void uHDLC_print(const uHDLC* frame) {
    printf("Address: %#x\n", frame->address);
    printf("Control: %#x\n", frame->control);
    printf("Data Length: %zu\n", frame->data_length);
    printf("Data: ");
    for (size_t i = 0; i < frame->data_length; i++)
    {
        printf("%c", frame->data[i]);
    }
    printf("\nCalculated FCS: %#x\n", frame->fcs);
    printf("Full Frame Length: %zu\n", frame->frame_length);
    printf("Full Frame: \n");
    frame_debugging(frame->frame, frame->frame_length);
}