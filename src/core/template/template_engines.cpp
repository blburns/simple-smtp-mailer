#include "simple-smtp-mailer/template_manager.hpp"
#include <sstream>

namespace ssmtp_mailer {

// Simple Template Engine
std::string SimpleTemplateEngine::render(const std::string& template_content, const TemplateContext& context) {
    std::string result = template_content;
    
    // Replace {{variable}} with actual values
    for (const auto& variable : context.variables) {
        std::string placeholder = "{{" + variable.name + "}}";
        size_t pos = 0;
        
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), variable.value);
            pos += variable.value.length();
        }
    }
    
    return result;
}

bool SimpleTemplateEngine::validate(const std::string& template_content) {
    // Check for balanced {{ }}
    int count = 0;
    for (size_t i = 0; i < template_content.length(); ++i) {
        if (template_content[i] == '{' && i + 1 < template_content.length() && template_content[i+1] == '{') {
            count++;
            i++;
        } else if (template_content[i] == '}' && i + 1 < template_content.length() && template_content[i+1] == '}') {
            count--;
            i++;
        }
    }
    return count == 0;
}

std::vector<std::string> SimpleTemplateEngine::extractVariables(const std::string& template_content) {
    std::vector<std::string> variables;
    std::regex var_regex(R"(\{\{(\w+)\}\})");
    std::smatch matches;
    
    std::string::const_iterator search_start(template_content.cbegin());
    while (std::regex_search(search_start, template_content.cend(), matches, var_regex)) {
        variables.push_back(matches[1].str());
        search_start = matches.suffix().first;
    }
    
    return variables;
}

// Handlebars Template Engine
std::string HandlebarsTemplateEngine::render(const std::string& template_content, const TemplateContext& context) {
    std::string result = template_content;
    
    // Process conditionals ({{#if variable}}...{{/if}})
    result = processConditionals(result, context);
    
    // Process loops ({{#each items}}...{{/each}})
    result = processLoops(result, context);
    
    // Process regular variables {{variable}}
    for (const auto& variable : context.variables) {
        std::string placeholder = "{{" + variable.name + "}}";
        size_t pos = 0;
        
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), variable.value);
            pos += variable.value.length();
        }
    }
    
    // Process partials and helpers would go here
    
    return result;
}

bool HandlebarsTemplateEngine::validate(const std::string& template_content) {
    // More complex validation for Handlebars
    return SimpleTemplateEngine::validate(template_content);
}

std::vector<std::string> HandlebarsTemplateEngine::extractVariables(const std::string& template_content) {
    // Extract both regular variables and block helpers
    std::vector<std::string> variables;
    std::regex var_regex(R"(\{\{(\#)?(\w+)\}\})");
    std::smatch matches;
    
    std::string::const_iterator search_start(template_content.cbegin());
    while (std::regex_search(search_start, template_content.cend(), matches, var_regex)) {
        variables.push_back(matches[2].str());
        search_start = matches.suffix().first;
    }
    
    return variables;
}

std::string HandlebarsTemplateEngine::processHandlebars(const std::string& content, const TemplateContext& context) {
    return content; // Simplified for now
}

std::string HandlebarsTemplateEngine::processConditionals(const std::string& content, const TemplateContext& context) {
    std::regex if_regex(R"(\{\{#if\s+(\w+)\}\}(.*?)\{\{/if\}\})", std::regex_constants::dotall);
    std::smatch matches;
    std::string result = content;
    
    auto search_start = result.cbegin();
    while (std::regex_search(search_start, result.cend(), matches, if_regex)) {
        std::string var_name = matches[1].str();
        std::string block_content = matches[2].str();
        
        // Check if variable exists and has truthy value
        bool show_block = false;
        for (const auto& var : context.variables) {
            if (var.name == var_name) {
                show_block = !var.value.empty() && var.value != "0" && var.value != "false";
                break;
            }
        }
        
        std::string replacement = show_block ? block_content : "";
        result.replace(search_start, search_start + matches.length(), replacement);
        search_start = result.cbegin() + std::distance(result.cbegin(), search_start);
    }
    
    return result;
}

std::string HandlebarsTemplateEngine::processLoops(const std::string& content, const TemplateContext& context) {
    // Simplified loop processing
    std::regex each_regex(R"(\{\{#each\s+(\w+)\}\}(.*?)\{\{/each\}\})", std::regex_constants::dotall);
    std::smatch matches;
    std::string result = content;
    
    auto search_start = result.cbegin();
    while (std::regex_search(search_start, result.cend(), matches, each_regex)) {
        std::string var_name = matches[1].str();
        std::string block_content = matches[2].str();
        
        // For now, just remove the loop syntax
        std::string replacement = ""; // In a full implementation, would iterate over array
        result.replace(search_start, search_start + matches.length(), replacement);
        search_start = result.cbegin() + std::distance(result.cbegin(), search_start);
    }
    
    return result;
}

// Template Engine Factory
std::shared_ptr<TemplateEngine> TemplateFactory::createEngine(const std::string& type) {
    std::string lower_type = type;
    std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(), ::tolower);
    
    if (lower_type == "simple") {
        return std::make_shared<SimpleTemplateEngine>();
    } else if (lower_type == "handlebars") {
        return std::make_shared<HandlebarsTemplateEngine>();
    } else {
        // Default to simple engine
        return std::make_shared<SimpleTemplateEngine>();
    }
}

std::vector<std::string> TemplateFactory::getSupportedEngines() {
    return {"Simple", "Handlebars"};
}

} // namespace ssmtp_mailer

