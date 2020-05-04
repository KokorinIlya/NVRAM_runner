#include <cstdio>
#include "../code/persistent_stack/persistent_stack.h"
#include "../code/persistent_stack/ram_stack.h"
#include "../code/persistent_stack/call.h"
#include "gtest/gtest.h"
#include "test_utils.h"

TEST(persistent_stack, add_frame)
{
    std::string temp_file_name = get_temp_file_name("stack");

    persistent_stack p_stack(temp_file_name, false);
    ram_stack r_stack;
    stack_frame frame_1 = stack_frame
            {
                    "some_function_name",
                    std::vector<uint8_t>({1, 3, 3, 7})
            };
    add_new_frame(r_stack, frame_1, p_stack);

    ram_stack another_r_stack = read_stack(p_stack);
    EXPECT_EQ(another_r_stack.size(), 1);
    stack_frame top_frame = another_r_stack.top().frame;
    EXPECT_EQ(top_frame.function_name, "some_function_name");
    EXPECT_EQ(top_frame.args, std::vector<uint8_t>({1, 3, 3, 7}));

    if (remove(temp_file_name.c_str()) != 0)
    {
        perror("Close file");
    }
}

TEST(persistent_stack, add_multiple_frames)
{
    std::string temp_file_name = get_temp_file_name("stack");

    persistent_stack p_stack(temp_file_name, false);
    ram_stack r_stack;
    stack_frame frame_1 = stack_frame
            {
                    "some_function_name",
                    std::vector<uint8_t>({1, 3, 3, 7})
            };
    add_new_frame(r_stack, frame_1, p_stack);
    stack_frame frame_2 = stack_frame
            {
                    "another_function_name",
                    std::vector<uint8_t>({2, 5, 1, 7})
            };
    add_new_frame(r_stack, frame_2, p_stack);
    stack_frame frame_3 = stack_frame
            {
                    "one_more_function_name",
                    std::vector<uint8_t>({1, 3, 5, 7, 9})
            };
    add_new_frame(r_stack, frame_3, p_stack);

    ram_stack another_r_stack = read_stack(p_stack);
    EXPECT_EQ(another_r_stack.size(), 3);

    stack_frame another_frame_3 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_3.function_name, "one_more_function_name");
    EXPECT_EQ(another_frame_3.args, std::vector<uint8_t>({1, 3, 5, 7, 9}));

    stack_frame another_frame_2 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_2.function_name, "another_function_name");
    EXPECT_EQ(another_frame_2.args, std::vector<uint8_t>({2, 5, 1, 7}));

    stack_frame another_frame_1 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_1.function_name, "some_function_name");
    EXPECT_EQ(another_frame_1.args, std::vector<uint8_t>({1, 3, 3, 7}));

    if (remove(temp_file_name.c_str()) != 0)
    {
        perror("Close file");
    }
}

TEST(persistent_stack, add_multiple_frames_remove_single_frame)
{
    std::string temp_file_name = get_temp_file_name("stack");

    persistent_stack p_stack(temp_file_name, false);
    ram_stack r_stack;
    stack_frame frame_1 = stack_frame
            {
                    "some_function_name",
                    std::vector<uint8_t>({1, 3, 3, 7})
            };
    add_new_frame(r_stack, frame_1, p_stack);
    stack_frame frame_2 = stack_frame
            {
                    "another_function_name",
                    std::vector<uint8_t>({2, 5, 1, 7})
            };
    add_new_frame(r_stack, frame_2, p_stack);
    stack_frame frame_3 = stack_frame
            {
                    "one_more_function_name",
                    std::vector<uint8_t>({1, 3, 5, 7, 9})
            };
    add_new_frame(r_stack, frame_3, p_stack);
    remove_frame(r_stack, p_stack);

    ram_stack another_r_stack = read_stack(p_stack);
    EXPECT_EQ(another_r_stack.size(), 2);

    stack_frame another_frame_2 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_2.function_name, "another_function_name");
    EXPECT_EQ(another_frame_2.args, std::vector<uint8_t>({2, 5, 1, 7}));

    stack_frame another_frame_1 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_1.function_name, "some_function_name");
    EXPECT_EQ(another_frame_1.args, std::vector<uint8_t>({1, 3, 3, 7}));

    if (remove(temp_file_name.c_str()) != 0)
    {
        perror("Close file");
    }
}

TEST(persistent_stack, add_and_remove_multiple_frames)
{
    std::string temp_file_name = get_temp_file_name("stack");

    persistent_stack p_stack(temp_file_name, false);
    ram_stack r_stack;
    stack_frame frame_1 = stack_frame
            {
                    "some_function_name",
                    std::vector<uint8_t>({1, 3, 3, 7})
            };
    add_new_frame(r_stack, frame_1, p_stack);
    stack_frame frame_2 = stack_frame
            {
                    "another_function_name",
                    std::vector<uint8_t>({2, 5, 1, 7})
            };
    add_new_frame(r_stack, frame_2, p_stack);
    remove_frame(r_stack, p_stack);
    stack_frame frame_3 = stack_frame
            {
                    "one_more_function_name",
                    std::vector<uint8_t>({1, 3, 5, 7, 9})
            };
    add_new_frame(r_stack, frame_3, p_stack);
    remove_frame(r_stack, p_stack);

    ram_stack another_r_stack = read_stack(p_stack);
    EXPECT_EQ(another_r_stack.size(), 1);

    stack_frame another_frame_1 = another_r_stack.top().frame;
    another_r_stack.pop();
    EXPECT_EQ(another_frame_1.function_name, "some_function_name");
    EXPECT_EQ(another_frame_1.args, std::vector<uint8_t>({1, 3, 3, 7}));

    if (remove(temp_file_name.c_str()) != 0)
    {
        perror("Close file");
    }
}

