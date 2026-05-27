# 🤝 Contributing to Flipper Next-Gen OS

Thank you for your interest in contributing to Flipper Next-Gen OS! 🎉

This document provides guidelines and information for contributors. Please read it carefully before submitting any contributions.

---

## 🚀 Quick Start for Contributors

### Ways to Contribute

1. **⭐ Star the repository** - Help us gain visibility!
2. **🐛 Report bugs** - Found an issue? Let us know!
3. **💡 Suggest features** - Have ideas? We want to hear them!
4. **🔧 Submit pull requests** - Code contributions are welcome!
5. **📖 Improve documentation** - Help us make docs better!
6. **🧪 Write tests** - Help us improve code quality!

---

## 🛠️ Development Setup

### Prerequisites

- **Hardware**: Flipper Zero device (recommended for testing)
- **OS**: Linux, macOS, or Windows 10+ with WSL2
- **Tools**: Git, Python 3.9+, ARM GCC toolchain, SCons

### Setup Instructions

```bash
# 1. Fork and clone the repository
git clone https://github.com/YOUR_USERNAME/flipper-nextgen-os.git
cd flipper-nextgen-os

# 2. Add upstream remote
git remote add upstream https://github.com/cedendahlkim/flipper-nextgen-os.git

# 3. Install dependencies
pip3 install -r requirements.txt

# 4. Install ARM toolchain
# Ubuntu/Debian:
sudo apt-get install gcc-arm-none-eabi scons
# macOS:
brew install arm-none-eabi-gcc scons

# 5. Build the firmware
scons -j$(nproc)
```

---

## 📝 Contribution Guidelines

### Code Style

#### C Code
- Follow Linux kernel coding style
- Use 4-space indentation (no tabs)
- Maximum line length: 120 characters
- Comments in Swedish for business logic, English for infrastructure

```c
// Exempel på svensk kommentar för affärslogik
bool bluetooth_scan_devices(void) {
    // Skanna efter BLE-enheter i närheten
    return scan_ble_devices();
}

/* Example English comment for infrastructure */
void furi_log_init(void) {
    // Initialize logging system
    log_system_init();
}
```

#### Python Code
- Follow PEP 8
- Use type hints
- Maximum line length: 120 characters
- Use Black for formatting

```python
def build_firmware(target: str, jobs: int = 4) -> bool:
    """Build firmware for specified target.
    
    Args:
        target: Build target (e.g., 'f7')
        jobs: Number of parallel jobs
        
    Returns:
        True if build successful
    """
    return scons_build(target, jobs)
```

#### Rust Code
- Use rustfmt for formatting
- Use clippy for linting
- Follow Rust API guidelines

```rust
/// AI-powered signal detector for protocol analysis
pub struct AiSignalDetector {
    model: Box<dyn MLModel>,
    confidence_threshold: f32,
}

impl AiSignalDetector {
    /// Analyze signal and detect protocol
    pub fn analyze_signal(&self, signal: &[u8]) -> Option<Protocol> {
        // Implementation here
    }
}
```

---

## 🌿 Branch Strategy

### Branch Naming

- `feature/feature-name` - New features
- `bugfix/bug-description` - Bug fixes
- `hotfix/critical-fix` - Critical fixes
- `docs/documentation-update` - Documentation updates

### Workflow

1. **Create a feature branch**
```bash
git checkout -b feature/your-feature-name
```

2. **Make your changes**
- Write clean, well-commented code
- Add tests for new functionality
- Update documentation

3. **Test your changes**
```bash
# Run tests
python -m pytest tests/

# Build firmware
scons -j$(nproc)

# Check code style
black --check .
flake8 .
```

4. **Commit your changes**
```bash
git add .
git commit -m "feat: add new protocol support"
```

5. **Push and create PR**
```bash
git push origin feature/your-feature-name
# Create Pull Request on GitHub
```

---

## 📋 Commit Message Guidelines

Use conventional commits format:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or updating tests
- `chore`: Build process, dependency updates

### Examples

```bash
feat(apps): add bluetooth scanner application
fix(core): resolve memory leak in gui system
docs(readme): update installation instructions
test(core): add unit tests for furi core
refactor(gui): simplify widget rendering code
```

---

## 🧪 Testing Guidelines

### Unit Tests

- Write tests for all new functions
- Use descriptive test names
- Test both success and failure cases

```python
def test_bluetooth_scan_devices_success():
    """Test successful Bluetooth device scanning."""
    scanner = BluetoothScanner()
    devices = scanner.scan_devices()
    assert isinstance(devices, list)
    assert len(devices) >= 0

def test_bluetooth_scan_devices_failure():
    """Test Bluetooth scan failure handling."""
    scanner = BluetoothScanner()
    with pytest.raises(BluetoothError):
        scanner.scan_devices(timeout=0)
```

### Integration Tests

- Test component interactions
- Use real hardware when possible
- Test end-to-end workflows

### Manual Testing

- Test on actual Flipper Zero hardware
- Verify GUI responsiveness
- Check battery impact

---

## 🐛 Bug Reports

### Bug Report Template

```markdown
## Bug Description
Brief description of the bug

## Steps to Reproduce
1. Step one
2. Step two
3. Step three

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- Flipper Zero firmware version: 
- Hardware version: 
- OS: 
- Browser (if applicable):

## Additional Context
Any additional information, screenshots, logs
```

---

## 💡 Feature Requests

### Feature Request Template

```markdown
## Feature Description
Clear description of the feature

## Problem Statement
What problem does this solve?

## Proposed Solution
How should this be implemented?

## Alternatives Considered
Other approaches you've thought about

## Additional Context
Any other relevant information
```

---

## 🔐 Security

### Security Issues

If you discover a security vulnerability, please:

1. **Do not** open a public issue
2. Email us at: gracestackab@gmail.com
3. Include details about the vulnerability
4. We'll respond within 48 hours

### Security Guidelines

- Follow secure coding practices
- Validate all inputs
- Use proper memory management
- Test for common vulnerabilities

---

## 📖 Documentation

### Documentation Types

- **API Documentation**: Code comments and docstrings
- **User Documentation**: README files and guides
- **Developer Documentation**: Architecture and setup guides

### Writing Guidelines

- Use clear, concise language
- Include code examples
- Add diagrams where helpful
- Keep documentation up to date

---

## 🎯 Review Process

### Pull Request Review

All PRs go through review:

1. **Automated Checks**
   - CI/CD pipeline
   - Code quality checks
   - Test coverage

2. **Manual Review**
   - Code review by maintainers
   - Architecture review for large changes
   - Security review for sensitive changes

3. **Approval Requirements**
   - At least one maintainer approval
   - All checks must pass
   - Documentation updated if needed

### Merge Strategy

- **Squash and merge** for most PRs
- **Rebase and merge** for feature branches
- **Merge commit** for hotfixes

---

## 🏆 Recognition

### Contributors Hall of Fame

Top contributors are recognized in:

- README.md
- Release notes
- Contributor statistics
- Special roles in the project

### Types of Contributions

- **Code**: Core functionality, apps, drivers
- **Documentation**: Guides, API docs, tutorials
- **Testing**: Test cases, bug reports
- **Community**: Support, discussions, outreach
- **Design**: UI/UX, graphics, icons

---

## 📞 Getting Help

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and ideas
- **Discord**: Real-time chat (link in README)
- **Email**: gracestackab@gmail.com

### Resources

- [API Documentation](docs/api.md)
- [Architecture Guide](docs/architecture.md)
- [Troubleshooting Guide](docs/troubleshooting.md)
- [Community Forum](https://forum.flipperzero.one)

---

## 📄 License

By contributing to this project, you agree that your contributions will be licensed under the GPL-3.0 License.

---

## 🙏 Thank You!

We appreciate all contributions, big or small! Whether you're fixing a typo, adding a feature, or just reporting a bug, you're helping make Flipper Next-Gen OS better for everyone.

**Together we're building the most advanced firmware for Flipper Zero! 🚀**

---

<div align="center">

**🌟 Happy Coding! 🌟**

</div>
