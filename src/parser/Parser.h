// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>

namespace vtparser
{

// NOLINTBEGIN(readability-identifier-naming)
enum class State : uint8_t
{
    /// Internal state to signal that this state doesn't exist (or hasn't been set).
    Undefined,

    /**
     * This is the initial state of the parser, and the state used to consume all characters
     * other than components of escape and control sequences.
     */
    Ground,

    /**
     * This state is entered whenever the C0 control ESC is received.
     *
     * This will immediately cancel any escape sequence,
     * control sequence or control string in progress.
     * If an escape sequence or control sequence was in progress,
     * “cancel” means that the sequence will have no effect,
     * because the final character that determines the control function
     * (in conjunction with any intermediates) will not have been received.
     * However, the ESC that cancels a control string may occur after
     * the control function has been determined and the following string
     * has had some effect on terminal state.
     *
     * For example, some soft characters may already have been defined.
     * Cancelling a control string does not undo these effects.
     */
    Escape,

    /**
     * This state is entered when an intermediate character arrives in an escape sequence.
     *
     * Escape sequences have no parameters, so the control function to be invoked is determined
     * by the intermediate and final characters.
     */
    EscapeIntermediate,

    /**
     * This state is entered when the control function CSI is recognised, in 7-bit or 8-bit form.
     *
     * This state will only deal with the first character of a control sequence, because
     * the characters 3C-3F can only appear as the first character of a control sequence, if
     * they appear at all.
     */
    CSI_Entry,

    /**
     * This state is entered when a parameter character is recognised in a control sequence.
     *
     * It then recognises other parameter characters until an intermediate or final character
     * appears. Further occurrences of the private-marker characters 3C-3F or the character 3A,
     * which has no standardised meaning, will cause transition to the csi ignore state.
     */
    CSI_Param,

    /**
     * This state is entered when an intermediate character is recognised in a control sequence.
     *
     * It then recognises other intermediate characters until a final character appears. If any
     * more parameter characters appear, this is an error condition which will cause a
     * transition to the csi ignore state.
     */
    CSI_Intermediate,

    /**
     * This state is used to consume remaining characters of a control sequence that is still
     * being recognised, but has already been disregarded as malformed.
     *
     * This state will only exit when a final character is recognised,
     * at which point it transitions to ground state
     * without dispatching the control function. This state may be entered because:
     *
     * 1.) a private-marker character 3C-3F is recognised in any place other than the first
     *     character of the control sequence,
     * 2.) the character 3A appears anywhere, or
     * 3.) a parameter character 30-3F occurs after an intermediate character has been recognised.
     *
     * C0 controls will still be executed while a control sequence is being ignored.
     */
    CSI_Ignore,

    /**
     * This state is entered when the control function DCS is recognised, in 7-bit or 8-bit form.
     *
     * X3.64 doesn’t define any structure for device control strings, but Digital made
     * them appear like control sequences followed by a data string, with a form and length
     * dependent on the control function. This state is only used to recognise the first
     * character of the control string, mirroring the csi entry state.
     *
     * C0 controls other than CAN, SUB and ESC are not executed while recognising the first part
     * of a device control string.
     */
    DCS_Entry,

    /**
     * This state is entered when a parameter character is recognised in a device control
     * string. It then recognises other parameter characters until an intermediate or final
     * character appears. Occurrences of the private-marker characters 3C-3F or the undefined
     * character 3A will cause a transition to the dcs ignore state.
     */
    DCS_Param,

    /**
     * This state is entered when an intermediate character is recognised in a device control
     * string. It then recognises other intermediate characters until a final character appears.
     * If any more parameter characters appear, this is an error condition which will cause a
     * transition to the dcs ignore state.
     */
    DCS_Intermediate,

    /**
     * This state is a shortcut for writing state machines for all possible device control
     * strings into the main parser. When a final character has been recognised in a device
     * control string, this state will establish a channel to a handler for the appropriate
     * control function, and then pass all subsequent characters through to this alternate
     * handler, until the data string is terminated (usually by recognising the ST control
     * function).
     *
     * This state has an exit action so that the control function handler can be informed when
     * the data string has come to an end. This is so that the last soft character in a DECDLD
     * string can be completed when there is no other means of knowing that its definition has
     * ended, for example.
     */
    DCS_PassThrough,

    /**
     * This state is used to consume remaining characters of a device control string that is
     * still being recognised, but has already been disregarded as malformed. This state will
     * only exit when the control function ST is recognised, at which point it transitions to
     * ground state. This state may be entered because:
     *
     * 1.) a private-marker character 3C-3F is recognised in any place other than the first
     *     character of the control string,
     * 2.) the character 3A appears anywhere, or
     * 3.) a parameter character 30-3F occurs after an intermediate character has been recognised.
     *
     * These conditions are only errors in the first part of the control string, until a final
     * character has been recognised. The data string that follows is not checked by this
     * parser.
     */
    DCS_Ignore,

    /**
     * This state is entered when the control function OSC (Operating System Command) is
     * recognised. On entry it prepares an external parser for OSC strings and passes all
     * printable characters to a handler function. C0 controls other than CAN, SUB and ESC are
     * ignored during reception of the control string.
     *
     * The only control functions invoked by OSC strings are DECSIN (Set Icon Name) and DECSWT
     * (Set Window Title), present on the multisession VT520 and VT525 terminals. Earlier
     * terminals treat OSC in the same way as PM and APC, ignoring the entire control string.
     */
    OSC_String,

    /**
     * Application Program Command.
     * ESC _ ... ST
     */
    APC_String,

    /*
     * Private Message
     * ESC ^ ... ST
     *
     * The payload need not to be printable characters.
     */
    PM_String,

    /**
     * The VT500 doesn’t define any function for these control strings, so this state ignores
     * all received characters until the control function ST is recognised.
     */
    IgnoreUntilST,
};
// NOLINTEND(readability-identifier-naming)

/// Actions can be invoked due to various reasons.
enum class ActionClass
{
    /// Action to be invoked because we enter a new state.
    Enter,
    /// Action to be invoked while not changing state.
    Event,
    /// Action to be invoked because we leave a state.
    Leave,
    /// Action to be invoked upon transitioning from one state to another.
    Transition,
};

/// An event may cause one of these actions to occur with or without a change of state.
enum class Action : uint8_t
{
    /// Internal state to signal that this action doesn't exist (or hasn't been assigned to).
    Undefined,

    /**
     * The character or control is not processed. No observable difference in the terminal’s
     * state would occur if the character that caused this action was not present in the input
     * stream. (Therefore, this action can only occur within a state.)
     */
    Ignore,

    /**
     * This action only occurs in ground state. The current code should be mapped to a glyph
     * according to the character set mappings and shift states in effect, and that glyph should
     * be displayed. 20 (SP) and 7F (DEL) have special behaviour in later VT series, as
     * described in ground.
     */
    Print,

    /**
     * This action only occurs in ground state and marks the end of printing characters.
     */
    PrintEnd,

    /**
     * The C0 or C1 control function should be executed, which may have any one of a variety of
     * effects, including changing the cursor position, suspending or resuming communications or
     * changing the shift states in effect. There are no parameters to this action.
     */
    Execute,

    /**
     * This action causes the current private flag, intermediate characters, final character and
     * parameters to be forgotten. This occurs on entry to the escape, csi entry and dcs entry
     * states, so that erroneous sequences like CSI 3 ; 1 CSI 2 J are handled correctly.
     */
    Clear,

    /**
     * The private marker or intermediate character should be stored for later use in selecting
     * a control function to be executed when a final character arrives. X3.64 doesn’t place any
     * limit on the number of intermediate characters allowed before a final character, although
     * it doesn’t define any control sequences with more than one. Digital defined escape
     * sequences with two intermediate characters, and control sequences and device control
     * strings with one. If more than two intermediate characters arrive, the parser can just
     * flag this so that the dispatch can be turned into a null operation.
     */
    Collect,

    /**
     * Collects the leading private marker, such as the '?' in `CSI ? Ps h`
     */
    CollectLeader,

    /**
     * This action collects the characters of a parameter string for a control sequence or
     * device control sequence and builds a list of parameters. The characters processed by this
     * action are the digits 0-9 (codes 30-39) and the semicolon (code 3B). The semicolon
     * separates parameters. There is no limit to the number of characters in a parameter
     * string, although a maximum of 16 parameters need be stored. If more than 16 parameters
     * arrive, all the extra parameters are silently ignored.
     */
    Param,
    ParamDigit,        // [0-9]
    ParamSeparator,    // ';'
    ParamSubSeparator, // ':'

    // NOLINTBEGIN(readability-identifier-naming)

    /**
     * The final character of an escape sequence has arrived, so determined the control function
     * to be executed from the intermediate character(s) and final character, and execute it.
     * The intermediate characters are available because collect stored them as they arrived.
     */
    ESC_Dispatch,

    /**
     * A final character has arrived, so determine the control function to be executed from
     * private marker, intermediate character(s) and final character, and execute it, passing in
     * the parameter list. The private marker and intermediate characters are available because
     * collect stored them as they arrived.
     */
    CSI_Dispatch,

    /**
     * This action is invoked when a final character arrives in the first part of a device
     * control string. It determines the control function from the private marker, intermediate
     * character(s) and final character, and executes it, passing in the parameter list. It also
     * selects a handler function for the rest of the characters in the control string. This
     * handler function will be called by the put action for every character in the control
     * string as it arrives.
     */
    Hook,

    /**
     * This action passes characters from the data string part of a device control string to a
     * handler that has previously been selected by the hook action. C0 controls are also passed
     * to the handler.
     */
    Put,

    /**
     * When a device control string is terminated by ST, CAN, SUB or ESC, this action calls the
     * previously selected handler function with an “end of data” parameter. This allows the
     * handler to finish neatly.
     */
    Unhook,

    APC_Start,
    APC_Put,
    APC_End,

    PM_Start,
    PM_Put,
    PM_End,

    /**
     * When the control function OSC (Operating System Command) is recognised,
     * this action initializes an external parser (the “OSC Handler”)
     * to handle the characters from the control string.
     *
     * OSC control strings are not structured in the same way as device control strings,
     * so there is no choice of parsers.
     */
    OSC_Start,

    /**
     * This action passes characters from the control string to the OSC Handler as they arrive.
     * There is therefore no need to buffer characters until the end of the control string is recognised.
     */
    OSC_Put,

    /**
     * This action is called when the OSC string is terminated by ST, CAN, SUB or ESC,
     * to allow the OSC handler to finish neatly.
     */
    OSC_End,

    /**
     * This action is called when Ground state is entered. The previous graphic character is then
     * being reset to 0 such that the grapheme cluster segmentation algorithm won't accidentally
     * mix up with older text.
     */
    GroundStart,
};
// NOLINTEND(readability-identifier-naming)

constexpr State& operator++(State& s) noexcept
{
    // NB: We allow to increment one element beyond maximum element (IgnoreUntilST) as sentinel
    // in order to allow easy iteration.
    if (s <= State::IgnoreUntilST)
        s = static_cast<State>(1 + static_cast<uint8_t>(s));

    return s;
}

constexpr std::string_view to_string(State state)
{
    switch (state)
    {
        case State::Undefined: return "Undefined";
        case State::Ground: return "Ground";
        case State::Escape: return "Escape";
        case State::EscapeIntermediate: return "EscapeIntermediate";
        case State::CSI_Entry: return "CSI Entry";
        case State::CSI_Param: return "CSI Param";
        case State::CSI_Intermediate: return "CSI Intermediate";
        case State::CSI_Ignore: return "CSI Ignore";
        case State::DCS_Entry: return "DCS Entry";
        case State::DCS_Param: return "DCS Param";
        case State::DCS_Intermediate: return "DCS Intermediate";
        case State::DCS_PassThrough: return "DCS PassThrough";
        case State::DCS_Ignore: return "DCS Ignore";
        case State::OSC_String: return "OSC String";
        case State::APC_String: return "APC String";
        case State::PM_String: return "PM String";
        case State::IgnoreUntilST: return "Ignore Until ST (SOS)";
    }
    return "?";
}

constexpr std::string_view to_string(ActionClass actionClass)
{
    switch (actionClass)
    {
        case ActionClass::Enter: return "Enter";
        case ActionClass::Event: return "Event";
        case ActionClass::Leave: return "Leave";
        case ActionClass::Transition: return "Transition";
    }
    return "?";
}

constexpr std::string_view to_string(Action action)
{
    switch (action)
    {
        case Action::Undefined: return "Undefined";
        case Action::GroundStart: return "GroundStart";
        case Action::Ignore: return "Ignore";
        case Action::Execute: return "Execute";
        case Action::Print: return "Print";
        case Action::PrintEnd: return "PrintEnd";
        case Action::Clear: return "Clear";
        case Action::Collect: return "Collect";
        case Action::CollectLeader: return "CollectLeader";
        case Action::Param: return "Param";
        case Action::ParamDigit: return "ParamDigit";
        case Action::ParamSeparator: return "ParamSeparator";
        case Action::ParamSubSeparator: return "ParamSubSeparator";
        case Action::ESC_Dispatch: return "Escape Dispatch";
        case Action::CSI_Dispatch: return "CSI Dispatch";
        case Action::Hook: return "Hook";
        case Action::Put: return "Put";
        case Action::Unhook: return "Unhook";
        case Action::OSC_Start: return "OSC Start";
        case Action::OSC_Put: return "OSC Put";
        case Action::OSC_End: return "OSC End";
        case Action::APC_Start: return "APC Start";
        case Action::APC_Put: return "APC Put";
        case Action::APC_End: return "APC End";
        case Action::PM_Start: return "PM Start";
        case Action::PM_Put: return "PM Put";
        case Action::PM_End: return "PM End";
    }
    return "?";
}

} // namespace vtparser

namespace std
{
template <>
struct numeric_limits<vtparser::State>
{
    using State = vtparser::State;
    constexpr static State min() noexcept { return State::Ground; } // skip Undefined
    constexpr static State max() noexcept { return State::IgnoreUntilST; }
    constexpr static size_t size() noexcept { return 17; }
};

template <>
struct numeric_limits<vtparser::Action>
{
    using Action = vtparser::Action;
    constexpr static Action min() noexcept { return Action::Ignore; } // skip Undefined
    constexpr static Action max() noexcept { return Action::OSC_End; }
    constexpr static size_t size() noexcept { return 19; }
};
} // namespace std

namespace vtparser
{

/**
 * Terminal Parser.
 *
 * Highly inspired by:
 *   https://vt100.net/emu/dec_ansi_parser
 *
 * The code comments for enum values have been mostly copied into this source for better
 * understanding when working with this parser.
 */
template <typename EventListener>
// TODO: C++20 concepts: EventListener must satisfy the original ParserEvents interface
class Parser
{
  public:
    friend EventListener;

    Parser() = default;

    explicit Parser(EventListener& listener): _eventListener { listener } {}

    using ParseError = std::function<void(std::string const&)>;
    using iterator = uint8_t const*;

    /**
     * Parses the input string in UTF-8 encoding and emits VT events while processing.
     *
     * @param data UTF-8 encoded string to be parsed.
     *
     * With respect to text, only up to @c EventListener::maxBulkTextSequenceWidth()
     * Unicode grapheme clusters will be processed.
     */
    void parseFragment(const std::string& data);

    State state() const noexcept { return _state; }

  private:
    enum class ProcessKind
    {
        /// Processing bulk-text in ground state succeed, keep on going.
        ContinueBulk,
        /// Processing bulk-text failed (various reasons), fallback to finit state machine.
        FallbackToFSM
    };

    std::tuple<ProcessKind, size_t> parseBulkText(char const* begin, char const* end) noexcept;
    void processOnceViaStateMachine(uint8_t ch);

    void handle(ActionClass actionClass, Action action, uint8_t codepoint);

    // private properties
    //
    State _state = State::Ground;
    EventListener& _eventListener;
};

} // end namespace vtparser

#include "Parser-impl.h"
